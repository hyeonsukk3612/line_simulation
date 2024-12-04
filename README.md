

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



