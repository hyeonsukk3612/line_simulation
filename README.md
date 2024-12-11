https://www.youtube.com/@newgull7736 혹시 동영상 오류시에는 유튜브 채널에서 찾아 보시면 되겠습니다!

7번영상 속도 50

https://github.com/user-attachments/assets/64889b39-101a-4a4d-84d7-7f00f54c1031

5번영상 속도 50

https://github.com/user-attachments/assets/ba0fa230-c5e9-4c4e-91b4-eba43ad39cba

7번영상 속도 100

https://github.com/user-attachments/assets/cc485a5d-7027-444f-bfca-8f8880abc364


5번영상 속도 100


https://github.com/user-attachments/assets/eb4ca89e-1e87-483f-b021-54291e95b321

주요 기능 부분분




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


전체적인 최종 코드에 대한 자세한 설명은 이렇게 됩니다.

최종 코드는 OpenCV와 Dynamixel 모터를 활용하여 실시간 영상 처리와 로봇 제어를 수행합니다. 

프로그램은 'Dxl' 클래스의 'mx' 객체를 생성하여 모터를 제어하고, 'ctrlc_handler' 함수로 SIGINT 시그널을 처리합니다. 

카메라 입력은 'VideoCapture' 객체 'source'를 통해 설정되며, 동영상 파일로는 '7.mp4' 파일을 사용합니다. 

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

실제 코스 도는 영상입니다!

[https://www.youtube.com/shorts/5SGmwgLo_hM](https://www.youtube.com/watch?v=35W5Sj6jz48)

프로젝트를 통해 느낀점 및 후기

이 프로젝트를 통해 실시간 영상 처리와 로봇 제어의 복잡성과 중요성을 깊이 이해하게 되었습니다.
OpenCV 라이브러리를 사용하여 이미지 처리 기법을 적용하고, 객체 검출 및 추적 알고리즘을 구현하는
과정에서 컴퓨터 비전의 실제 응용에 대한 통찰을 얻었습니다. 특히 연결 요소 분석을 통한 객체 검출과
최근 객체 추적 방법은 실제 환경에서의 로봇 제어에 매우 유용함을 체감했습니다. 하지만
수업 시간에만 사용할 수 있어서 오래 사용하지 못했고 수업에 필요한 작업만 해서 아쉬웠습니다.
또한, Dynamixel 모터 제어를 통해 하드웨어와 소프트웨어의 통합이 얼마나 중요한지 깨달았습니다.
프로그램 개발 과정에서 실시간 성능과 정확성 사이의 균형을 맞추는 것이 큰 도전이었습니다.
영상 처리, 객체 검출, 모터 제어 등 여러 작업을 동시에 수행하면서도 실시간성을 유지해야 했기 때문에,
효율적인 알고리즘 설계와 최적화의 중요성을 실감했습니다. 이런 작업을 할 때는 좋은 GPU가 필요한가에 대해 궁금한 적이 있는데
이번 프로젝트를 통해 멀티스레딩이나 GPU 가속과 같은 고급 기술의 필요성도 인식하게 되었습니다.
이 프로젝트는 로보틱스, 컴퓨터 비전, 실시간 시스템 제어 등 다양한 분야의 지식을 통합적으로 적용해야 하는 복잡한 과제였습니다.
이를 통해 학제 간 접근의 중요성과 각 분야 간의 상호작용을 이해하게 되었습니다.
또한, 실제 환경에서의 불확실성과 변동성을 다루는 것의 어려움을 체감하며, 견고한 시스템 설계의 중요성을 깨달았습니다.
그리고 항상 코드로만 작성했는데 이건 실제로 작동하는 것이 눈으로 관찰할 수 있어서 나중에 이런 일 분야는
이런 식으로 하는 기본 이론과 작동 원리를 기본적으로 숙지하고 있어야 하는 이유를 알게 된 것 같습니다.
특히 빛 반사로 인한 라인 검출의 방해는 예상치 못한 오류였습니다. 생각보다 당시 상황이 되면 해결하기 어려우며,
같은 부분에서 작동이 안 된다는 점으로 빠르게 파악해야 했지만, 경험 부족으로 당시에는 쉽게 알아내기 어려웠었습니다.
그리고 200rpm 부분에서 코스를 돌긴하지만 안정적으로 돌지 못하는 점은 아쉬웠습니다.
이 프로젝트는 로봇 공학과 컴퓨터 비전 분야에서의 실제적인 문제 해결 능력을 크게 향상된 것 같으며,
앞으로의 진로와 직업에 관해 큰 동기부여가 되었습니다.


