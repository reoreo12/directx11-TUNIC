# directx11-TUNIC
C++ DirectX 11 기반 3D 액션 어드벤처 게임 'TUNIC' 모작

<p align="center">
    <img src="Images/TUNIC_1.jpg" width="45%" alt="튜닉 모작 스크린샷1">
    <img src="Images/TUNIC_5.jpg" width="45%" alt="튜닉 모작 스크린샷2"><br>
    <img src="Images/TUNIC_9.jpg" width="45%" alt="튜닉 모작 스크린샷3">
    <img src="Images/TUNIC_10.jpg" width="45%" alt="튜닉 모작 스크린샷4"><br>
    <img src="Images/TUNIC_11.jpg" width="45%" alt="튜닉 모작 스크린샷5">
    <img src="Images/TUNIC_13.jpg" width="45%" alt="튜닉 모작 스크린샷6"><br>
    <img src="Images/TUNIC_17.jpg" width="45%" alt="튜닉 모작 스크린샷7">
    <img src="Images/TUNIC_18.jpg" width="45%" alt="튜닉 모작 스크린샷8"><br>
    <img src="Images/TUNIC_24.jpg" width="45%" alt="튜닉 모작 스크린샷9">
    <img src="Images/TUNIC_25.jpg" width="45%" alt="튜닉 모작 스크린샷10">

</p>

<p align="center">
    <a href="https://youtu.be/qXo6ew3sXlM">
        <img src="https://img.shields.io/badge/YouTube-red?logo=youtube&logoColor=white&style=for-the-badge" alt="YouTube 플레이 영상">
    </a>
</p>

## 대표 클래스 위치
아래 토글에서 각 클래스의 **프로젝트 내 위치/역할/조각 코드/설명** 등을 볼 수 있습니다.

<details>
<summary><code>CCollider/CCollision_Manager</code></summary>

- **역할**
    - CCollider: AABB/OBB/Sphere 등 충돌체를 표현하는 컴포넌트
    - CCollision_Manager: 객체 간 충돌을 검사하고 충돌 이벤트를 호출하는 매니저 클래스

- **실제 프로젝트 경로**
    - Engine/Public/Collider.h
    - Engine/Private/Collider.cpp

    - Engine/Public/Collision_Manager.h
    - Engine/Private/Collision_Manager.cpp

- **소스 바로가기**
    - [Collider.h](Collision/Collider/Collider.h)
    - [Collider.cpp](Collision/Collider/Collider.cpp)

    - [Collision.h](Collision/Collision_Manager/Collision_Manager.h)
    - [Collision.cpp](Collision/Collision_Manager/Collision_Manager.cpp)

</details>