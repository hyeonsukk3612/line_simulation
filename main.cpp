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

    // 카메라 설정 (주석 처리됨)
    //VideoCapture source(src, CAP_GSTREAMER);
    VideoCapture source("7.mp4");  // 비디오 파일 열기
    if (!source.isOpened()){ cout << "Camera error" << endl; return -1; }
    
    // 비디오 스트리밍 설정
    VideoWriter writer(dst, 0, (double)30, Size(640,360), true);
    if(!writer.isOpened()) {
        cerr << "Writer open failed!" << endl;
        return -1;
    }

    // 처리된 영상 스트리밍 설정
    VideoWriter writer1(dst1, 0, (double)30, Size(640,90), true);
    if(!writer1.isOpened()) {
        cerr << "Writer1 open failed!" << endl;
        return -1;
    }

    Mat frame, gray, binary;
    Point mainPoint;  // 주 추적 포인트
    Point lastPosition;  // 마지막 감지 위치
    bool mode = true;  // 초기화 모드

    while (!ctrl_c_pressed) {
        gettimeofday(&start, NULL);  // 시작 시간 기록

        source >> frame;  // 프레임 읽기
        if (frame.empty()) break;

        cvtColor(frame, gray, COLOR_BGR2GRAY);  // 그레이스케일 변환
        gray += Scalar(100) - mean(gray);  // 밝기 조정
        threshold(gray, binary, 128, 255, THRESH_BINARY);  // 이진화

        Rect roi(0, max(0, binary.rows - 90), binary.cols, 90);  // ROI 설정
        binary = binary(roi);

        if (mode) {
            mainPoint = Point(binary.cols / 2, binary.rows - 1);  // 초기 주 포인트 설정
            lastPosition = mainPoint;
            mode = false;
        }

        Mat label, stats, centroids, output;
        int num = connectedComponentsWithStats(binary, label, stats, centroids);  // 연결 요소 분석
        int closestIndex = -1;
        int minDist = binary.cols;

        cvtColor(binary, output, COLOR_GRAY2BGR);  // 출력용 이미지 생성

        // 연결 요소 분석 및 가장 가까운 객체 찾기
        for (int i = 1; i < num; i++) {
            int area = stats.at<int>(i, CC_STAT_AREA);
            if (area > 120) {
                Point center(cvRound(centroids.at<double>(i, 0)), cvRound(centroids.at<double>(i, 1)));
                Rect object(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP),
                            stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_HEIGHT));
                int dist = norm(center - mainPoint);
                if (dist <= 140 && dist < minDist) {
                    minDist = dist;
                    closestIndex = i;
                }
                
                rectangle(output, object, Scalar(255, 0, 0), 2);  // 모든 객체 파란색으로 표시
                circle(output, center, 5, Scalar(255, 0, 0), -1);
            }
        }

        // 가장 가까운 객체 처리
        if (closestIndex >= 0) {
            Rect object(stats.at<int>(closestIndex, CC_STAT_LEFT), stats.at<int>(closestIndex, CC_STAT_TOP),
                        stats.at<int>(closestIndex, CC_STAT_WIDTH), stats.at<int>(closestIndex, CC_STAT_HEIGHT));
            Point center(cvRound(centroids.at<double>(closestIndex, 0)), cvRound(centroids.at<double>(closestIndex, 1)));
            
            rectangle(output, object, Scalar(0, 0, 255), 2);  // 가장 가까운 객체 빨간색으로 표시
            circle(output, center, 5, Scalar(0, 0, 255), -1);
            mainPoint = center;
            lastPosition = center;
        }

        circle(output, lastPosition, 7, Scalar(0, 0, 255), -1);  // 마지막 위치 표시

        double error = output.cols/2 - lastPosition.x;  // 중앙으로부터의 오차 계산
        cout << "error: " << error << endl;

        int baseSpeed = 100;  // 기본 속도

        // 모터 속도 계산
        vel1 = baseSpeed - 0.1 * error;
        vel2 = -(baseSpeed + 0.1 * error);

        // 속도 제한
        vel1 = max(-200, min(200, vel1));
        vel2 = max(-200, min(200, vel2));

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
