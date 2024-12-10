#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include "dxl.hpp"

using namespace std;
using namespace cv;

bool ctrl_c_pressed = false;
void ctrlc_handler(int){ ctrl_c_pressed = true; }

int main() {
    Dxl mx;
    struct timeval start, end1;
    double time1;
    int vel1 = 0, vel2 = 0;

    signal(SIGINT, ctrlc_handler);
    if(!mx.open()) { cout << "dynamixel open error" << endl; return -1; }

    // 카메라 설정
    string src = "nvarguscamerasrc sensor-id=0 ! \
        video/x-raw(memory:NVMM), width=(int)640, height=(int)360, \
        format=(string)NV12, framerate=(fraction)30/1 ! \
        nvvidconv flip-method=0 ! video/x-raw, \
        width=(int)640, height=(int)360, format=(string)BGRx ! \
        videoconvert ! video/x-raw, format=(string)BGR ! appsink";
        
    //VideoCapture source(src, CAP_GSTREAMER);
    VideoCapture source("7.mp4");
    if (!source.isOpened()){ cout << "Camera error" << endl; return -1; }
    
    // 비디오 스트리밍 설정
    string dst = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
        nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! \
        h264parse ! rtph264pay pt=96 ! \
        udpsink host=203.234.58.168 port=8001 sync=false";

    VideoWriter writer(dst, 0, (double)30, Size(640,360), true);
    if(!writer.isOpened()) {
        cerr << "Writer open failed!" << endl;
        return -1;
    }

    // 처리된 이미지 스트리밍 설정
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
    Point mainPoint;
    Point lastPosition;
    bool mode = true;

    while (!ctrl_c_pressed) {
        gettimeofday(&start, NULL);

        source >> frame;
        if (frame.empty()) break;

        // 이미지 전처리
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        gray += Scalar(100) - mean(gray);
        threshold(gray, binary, 128, 255, THRESH_BINARY);

        // ROI 설정
        Rect roi(0, max(0, binary.rows - 90), binary.cols, 90);
        binary = binary(roi);

        if (mode) {
            mainPoint = Point(binary.cols / 2, binary.rows - 1);
            lastPosition = mainPoint;
            mode = false;
        }

        // 연결된 컴포넌트 분석
        Mat label, stats, centroids, output;
        int num = connectedComponentsWithStats(binary, label, stats, centroids);
        int closestIndex = -1;
        int minDist = binary.cols;

        cvtColor(binary, output, COLOR_GRAY2BGR);

        // 객체 검출 및 그리기
        for (int i = 1; i < num; i++) {
            int area = stats.at<int>(i, CC_STAT_AREA);
            if (area > 100) {
                Point center(cvRound(centroids.at<double>(i, 0)), cvRound(centroids.at<double>(i, 1)));
                Rect object(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP),
                            stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_HEIGHT));
                int dist = norm(center - mainPoint);
                if (dist <= 80 && dist < minDist) {
                    minDist = dist;
                    closestIndex = i;
                }
                
                rectangle(output, object, Scalar(255, 0, 0), 2);
                circle(output, center, 5, Scalar(255, 0, 0), -1);
            }
        }

        // 가장 가까운 객체 표시
        if (closestIndex >= 0) {
            Rect object(stats.at<int>(closestIndex, CC_STAT_LEFT), stats.at<int>(closestIndex, CC_STAT_TOP),
                        stats.at<int>(closestIndex, CC_STAT_WIDTH), stats.at<int>(closestIndex, CC_STAT_HEIGHT));
            Point center(cvRound(centroids.at<double>(closestIndex, 0)), cvRound(centroids.at<double>(closestIndex, 1)));
            
            rectangle(output, object, Scalar(0, 0, 255), 2);
            circle(output, center, 5, Scalar(0, 0, 255), -1);
            mainPoint = center;
            lastPosition = center;
        }

        circle(output, lastPosition, 7, Scalar(0, 0, 255), -1);

        // 모터 제어를 위한 에러 계산
        double error = output.cols/2 - lastPosition.x;
        cout << "error: " << error << endl;

        int baseSpeed = 100;

        // 모터 속도 계산
        vel1 = baseSpeed - 0.2 * error;
        vel2 = -(baseSpeed + 0.2 * error);

        // 속도 제한
        vel1 = max(-200, min(200, vel1));
        vel2 = max(-200, min(200, vel2));

        // 모터 속도 설정
        mx.setVelocity(vel1, vel2);

        // 비디오 스트리밍
        writer << frame;
        resize(output, output, Size(640, 90));
        writer1 << output;

        // 실행 시간 측정
        gettimeofday(&end1, NULL);
        usleep(20*1000);

        time1 = end1.tv_sec - start.tv_sec + (end1.tv_usec - start.tv_usec) / 1000000.0;
        cout << "vel1:" << vel1 << "," << "vel2:" << vel2 << ",time:" << time1 << endl;
    }

    mx.close();
    return 0;
}
