# code/ — 예제 코드 submodule 자리

예제 코드 저장소가 확정되면 이 디렉토리를 submodule 로 연결한다.

```bash
git rm -r --cached code && rm -rf code
git submodule add <예제 코드 저장소 URL> code
```

장별 구성 예정:

| 장 | 디렉토리 | 내용 |
|----|----------|------|
| 3장 | `ch03/` | 첫 패키지, 퍼블리셔·서브스크라이버 |
| 4장 | `ch04/` | 사용자 정의 인터페이스, 카메라·관절 상태 노드 |
| 5장 | `ch05/` | SO-ARM 101 URDF·xacro, RViz2 설정 |
| 6장 | `ch06/` | ros2_control 하드웨어 인터페이스, 컨트롤러 설정 |
| 7장 | `ch07/` | Gazebo 월드·런치 파일 |
| 8장 | `ch08/` | MoveIt 2 설정과 집기 예제 |
| 9장 | `ch09/` | 텔레오퍼레이션 노드, rosbag2 → LeRobotDataset 변환 |
| 10장 | `ch10/` | SmolVLA 파인튜닝 스크립트 |
| 11장 | `ch11/` | VLA 정책 노드, 런치 구성 |
| 12장 | `ch12/` | 실기 배포·평가 노드 |
