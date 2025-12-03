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
<summary><code>Collision</code></summary>

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

<details>
<summary><code>Climb</code></summary>

- **역할**
    - PlayerState_Climb: FSM에서 플레이어가 사다리에 탑승하는 상태 클래스

- **실제 프로젝트 경로**
    - Client/Public/PlayerState_Climb.h
    - Client/Private/PlayerState_Climb.cpp

- **소스 바로가기**
    - [PlayerState_Climb.h](Climb/PlayerState_Climb.h)
    - [PlayerState_Climb.cpp](Climb/PlayerState_Climb.cpp)

<details>
<summary>원리 설명</summary>

```cpp
// 사다리 위(LadderTop)·아래(LadderBottom) 셀에서의 탑승을 전부 고려한다
_bool bFoundTop = false;
_bool bFoundBottom = false;
```
<p align="center">
    <img src="Images/Climb_1.jpg" width="60%" alt="사다리 아래에서 타는 경우">
</p>
<p align="center">플레이어가 아래에서 사다리를 타는 경우</p>
<p align="center">
    <img src="Images/Climb_2.jpg" width="60%" alt="사다리 위에서 타는 경우">
</p>
<p align="center">플레이어가 위에서 사다리를 타는 경우</p>

<p align="center">
    <img src="Images/Climb_3.jpg" width="60%" alt="사다리 위에서 타는 경우의 플레이어">
</p>
<p align="center">플레이어가 위에서 타는 경우 몸을 돌리기</p>

```cpp
// 사다리의 노말 벡터를 반전시켜 플레이어가 바라볼 방향을 정한다
XMStoreFloat3(&m_vClimbDir, XMVector3Normalize(XMVectorNegate(vBestMiddleNormal)));

_vector vClimbDir = XMLoadFloat3(&m_vClimbDir);

// 플레이어가 바라볼 방향에 맞춰 Right, Up, Look 벡터 갱신
_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
_vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vClimbDir));
vUp = XMVector3Normalize(XMVector3Cross(vClimbDir, vRight));
```
<p align="center">
    <img src="Images/Climb_3.jpg" width="60%" alt="사다리 위에서 타는 경우의 플레이어">
</p>
<p align="center">플레이어의 Look 벡터와 사다리의 법선 벡터가 반대</p>

</details>

</details>