https://www.youtube.com/@newgull7736 혹시 동영상 오류시에는 유튜브 채널에서 찾아 보시면 되겠습니다!

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


OpenCV와 Dynamixel 모터를 활용하여 실시간 영상 처리와 로봇 제어를 수행합니다. 
프로그램은 'Dxl' 클래스의 'mx' 객체를 생성하여 모터를 제어하고, 'ctrlc_handler' 함수로 SIGINT 시그널을 처리합니다. 
카메라 입력은 'VideoCapture' 객체 'source'를 통해 설정되며, 실제로는 '7.mp4' 파일을 사용합니다. 
'VideoWriter' 객체 'writer'와 'writer1'은 각각 원본 영상과 처리된 영상을 스트리밍합니다. 
메인 루프에서는 'frame'을 캡처하고, 'cvtColor'와 'threshold' 함수로 전처리합니다. 
'connectedComponentsWithStats' 함수로 객체를 검출하고, 
가장 가까운 객체를 찾아 'mainPoint'와 'lastPosition'을 업데이트합니다.
검출된 객체는 'rectangle'과 'circle' 함수로 시각화됩니다. 객체 위치에 따른 에러값을 계산하여 
'vel1'과 'vel2' 변수로 모터 속도를 결정하고, 'mx.setVelocity' 함수로 모터를 제어합니다. 
'gettimeofday' 함수로 실행 시간을 측정하고 출력합니다. 
프로그램은 ROI(Region of Interest) 설정, 이진화, 연결 요소 분석 등의 이미지 처리 기법을 사용하며, 
객체 추적과 모터 제어를 결합하여 자율 주행 로봇이나 객체 추적 시스템의 기본 프레임워크를 제공합니다. 
'ctrl_c_pressed' 변수를 통해 프로그램의 안전한 종료를 보장하며, 루프 내에서 'usleep' 함수로 실행 주기를 조절합니다. 
전체적으로 이 코드는 컴퓨터 비전, 로보틱스, 실시간 시스템 제어를 통합하여 복잡한 로봇 제어 시스템의 기초를 구현하고 있습니다.
