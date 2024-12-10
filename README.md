

리눅스를 통한 카메라 화면 영상

https://github.com/user-attachments/assets/995ea673-557a-46b4-a09f-090cd0f36335

이 코드는 NVIDIA Jetson 보드에서 카메라를 사용하여 실시간으로 영상을 캡처하고, 이를 처리하여 라인을 따라가는 로봇을 제어하는 프로그램입니다. 

주요 기능으로는 GStreamer를 이용한 비디오 스트리밍, OpenCV를 사용한 이미지 처리, 그리고 Dynamixel 모터 제어가 포함됩니다. 

프로그램은 카메라에서 프레임을 캡처한 후, 그레이스케일 변환과 이진화를 거쳐 라인을 감지합니다. 

연결 요소 분석을 통해 가장 가까운 라인 객체를 찾고, 이를 기준으로 로봇의 주행 방향을 결정합니다.

라인 감지 후, 프로그램은 화면 중앙과 감지된 라인 사이의 오차를 계산하여 모터의 속도와 방향을 제어합니다. 

오차가 작으면 직진, 오른쪽으로 치우쳐 있으면 우회전, 왼쪽으로 치우쳐 있으면 좌회전하도록 모터 속도를 설정합니다. 

처리된 영상은 두 개의 별도 스트림으로 전송되며, 하나는 원본 프레임을, 다른 하나는 처리된 이미지를 보여줍니다. 

프로그램은 Ctrl+C 신호를 받을 때까지 계속 실행되며, 실행 중 각 프레임 처리에 걸린 시간과 설정된 모터 속도를 출력합니다

7번영상 속도 50

https://github.com/user-attachments/assets/64889b39-101a-4a4d-84d7-7f00f54c1031

5번영상 속도 50

https://github.com/user-attachments/assets/ba0fa230-c5e9-4c4e-91b4-eba43ad39cba

7번영상 속도 100

https://github.com/user-attachments/assets/cc485a5d-7027-444f-bfca-8f8880abc364


5번영상 속도 100


https://github.com/user-attachments/assets/eb4ca89e-1e87-483f-b021-54291e95b321




        // 연결 요소 분석 및 가장 가까운 객체 찾기
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
                
                rectangle(output, object, Scalar(255, 0, 0), 2);  // 모든 객체 파란색으로 표시
                circle(output, center, 5, Scalar(255, 0, 0), -1);
            }
        }
90크기 보다 큰 값을 안지하여 가운데 부분을 레이블링하여 파랑색으로 표시합니다. 그중에 100이하이고 제일 가까운 값을 저장합니다.


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

closestIndex값을 인지하면 가장 가까운 값을 위에서 받을 걸 기준으로 하여 빨강색으로 표시하고 그부분을 빨강색 레이블링 박스를 만들어 표시하고 박스의 중심점을 빨간색으로 표시합니다.


