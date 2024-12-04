#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include "dxl.hpp"

using namespace std;
using namespace cv;

bool ctrl_c_pressed = false;
void ctrlc_handler(int){ ctrl_c_pressed = true; }  // Ctrl+C 시그널 핸들러

int main() {
    Dxl mx;  // Dynamixel 모터 제어 객체
    struct timeval start, end1;
    double time1;
    int vel1 = 0, vel2 = 0;  // 좌우 모터 속도

    signal(SIGINT, ctrlc_handler);  // Ctrl+C 시그널 핸들러 등록
    if(!mx.open()) { cout << "dynamixel open error" << endl; return -1; }

    // NVIDIA Jetson 카메라에서 영상을 캡처하는 GStreamer 파이프라인
    string src = "nvarguscamerasrc sensor-id=0 ! \
        video/x-raw(memory:NVMM), width=(int)640, height=(int)360, \
        format=(string)NV12, framerate=(fraction)30/1 ! \
        nvvidconv flip-method=0 ! video/x-raw, \
        width=(int)640, height=(int)360, format=(string)BGRx ! \
        videoconvert ! video/x-raw, format=(string)BGR ! appsink";
        
    //VideoCapture source(src, CAP_GSTREAMER);
    VideoCapture source("5.mp4");  // 비디오 파일에서 영상 캡처
    if (!source.isOpened()){ cout << "Camera error" << endl; return -1; }
    
    // 원본 영상 스트리밍을 위한 GStreamer 파이프라인
    string dst = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
        nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! \
        h264parse ! rtph264pay pt=96 ! \
        udpsink host=203.234.58.168 port=8001 sync=false";

    VideoWriter writer(dst, 0, (double)30, Size(640,360), true);
    if(!writer.isOpened()) {
        cerr << "Writer open failed!" << endl;
        return -1;
    }

    // 처리된 영상 스트리밍을 위한 GStreamer 파이프라인
    string dst1 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
        nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! \
        h264parse ! rtph264pay pt=96 ! \
        udpsink host=203.234.58.168 port=8002 sync=false";

    VideoWriter writer1(dst1, 0, (double)30, Size(640,90), true);
    if(!writer1.isOpened()) {
        cerr << "Writer1 open failed!" << endl;
        return -1;
    }

    Mat frame, gray, binary;
    Point mainPoint;  // 현재 주행 기준점
    Point lastPosition;  // 마지막으로 감지된 위치
    bool mode = true;  // 초기화 모드 플래그

    while (!ctrl_c_pressed) {
        gettimeofday(&start, NULL);  // 시작 시간 기록

        source >> frame;  // 비디오에서 프레임 읽기
        if (frame.empty()) break;

        cvtColor(frame, gray, COLOR_BGR2GRAY);  // 그레이스케일 변환
        gray += Scalar(100) - mean(gray);  // 밝기 조정
        threshold(gray, binary, 128, 255, THRESH_BINARY);  // 이진화

        Rect roi(0, max(0, binary.rows - 90), binary.cols, 90);  // ROI 설정
        binary = binary(roi);

        if (mode) {
            mainPoint = Point(binary.cols / 2, binary.rows - 1);  // 초기 주행점 설정
            lastPosition = mainPoint;  // 초기 위치 설정
            mode = false;
        }

        Mat label, stats, centroids, output;
        int num = connectedComponentsWithStats(binary, label, stats, centroids);  // 연결 요소 분석
        int closestIndex = -1;
        int minDist = binary.cols;

        cvtColor(binary, output, COLOR_GRAY2BGR);  // 출력용 이미지 생성

        for (int i = 1; i < num; i++) {
            int area = stats.at<int>(i, CC_STAT_AREA);
            if (area > 120) {  // 일정 크기 이상의 객체만 처리
                Point center(cvRound(centroids.at<double>(i, 0)), cvRound(centroids.at<double>(i, 1)));
                Rect object(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP),
                            stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_HEIGHT));
                int dist = norm(center - mainPoint);
                if (dist <= 140 && dist < minDist) {
                    minDist = dist;
                    closestIndex = i;
                }
                
                // 모든 후보를 파란색으로 표시
                rectangle(output, object, Scalar(255, 0, 0), 2);
                circle(output, center, 5, Scalar(255, 0, 0), -1);
            }
        }

        if (closestIndex >= 0) {
            Rect object(stats.at<int>(closestIndex, CC_STAT_LEFT), stats.at<int>(closestIndex, CC_STAT_TOP),
                        stats.at<int>(closestIndex, CC_STAT_WIDTH), stats.at<int>(closestIndex, CC_STAT_HEIGHT));
            Point center(cvRound(centroids.at<double>(closestIndex, 0)), cvRound(centroids.at<double>(closestIndex, 1)));
            
            // 주행라인(가장 가까운 객체)을 빨간색으로 표시
            rectangle(output, object, Scalar(0, 0, 255), 2);
            circle(output, center, 5, Scalar(0, 0, 255), -1);
            mainPoint = center;
            lastPosition = center;  // 마지막 위치 업데이트
        }

        // 마지막 위치를 빨강색으로 표시
        circle(output, lastPosition, 7, Scalar(0, 0, 255), -1);

        double error = output.cols/2 - lastPosition.x;  // 중앙선으로부터의 오차 계산
        cout << "error: " << error << endl;

        int baseSpeed = 50;  // 기본 속도 설정

        // PID 제어 없이 간단한 비례 제어로 속도 조절
        vel1 = baseSpeed - 0.1 * error;
        vel2 = -(baseSpeed + 0.1 * error);

        // 속도 제한
        vel1 = max(-100, min(100, vel1));
        vel2 = max(-100, min(100, vel2));

        mx.setVelocity(vel1, vel2);  // 모터 속도 설정

        writer << frame;  // 원본 프레임 스트리밍
        resize(output, output, Size(640, 90));  // 출력 이미지 크기 조정
        writer1 << output;  // 처리된 이미지 스트리밍

        usleep(20*1000);  // 20ms 대기
        gettimeofday(&end1, NULL);  // 종료 시간 기록
        time1 = end1.tv_sec - start.tv_sec + (end1.tv_usec - start.tv_usec) / 1000000.0;
        cout << "vel1:" << vel1 << "," << "vel2:" << vel2 << ",time:" << time1 << endl;
    }

    mx.close();  // Dynamixel 모터 연결 종료
    return 0;
}
