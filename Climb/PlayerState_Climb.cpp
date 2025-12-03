#include "PlayerState_Climb.h"
#include "GameInstance.h"
#include "Player.h"
#include "Body_Player.h"
#include "Cell.h"

CPlayerState_Climb::CPlayerState_Climb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CGameObject* pOwner)
	: CState{ pDevice, pContext, pOwner }
{
}

CPlayerState_Climb::CPlayerState_Climb(const CState& Prototype)
	: CState { Prototype }
{
}

void CPlayerState_Climb::Enter_State()
{
	m_pFSM = m_pOwner->Get_Component<CFSM>(L"Com_FSM");
	NULL_CHECK(m_pFSM);

    m_pTransform = m_pOwner->Get_Component<CTransform>(L"Com_Transform");
    NULL_CHECK(m_pTransform);
    
    m_pNavigation = nullptr;

    // 현재 레벨에 맞는 내비게이션 선택
    if (LEVEL_GAMEPLAY == m_pGameInstance->Get_CurrentLevelIndex())
        m_pNavigation = m_pOwner->Get_Component<CNavigation>(L"Com_Navigation_EastForest");
    else if (LEVEL_BOSS == m_pGameInstance->Get_CurrentLevelIndex())
        m_pNavigation = m_pOwner->Get_Component<CNavigation>(L"Com_Navigation_BossMap");

    NULL_CHECK(m_pNavigation);

    if (m_pNavigation)
    {
        // 사다리 탑승 단계
        // 사다리 위(LadderTop)·아래(LadderBottom) 셀에서의 탑승을 전부 고려한다
        _bool bFoundTop = false;
        _bool bFoundBottom = false;

        // 현재 플레이어의 위치
        _vector vPlayerPos = m_pTransform->Get_State(CTransform::STATE_POSITION);

        // 현재 플레이어가 밟고 있는 셀의 인덱스 검색
        const auto& vecCells = m_pNavigation->Get_Cells();
        _int iCurrIdx = m_pNavigation->Get_CurrentCellIndex();

        _float fClosestDistSq = FLT_MAX;
        _vector vBestMiddleNormal = XMVectorSet(0.f, 0.f, 1.f, 0.f);

        // LadderTop·LadderBottom 셀의 Y 값 전부를 탑승한 Y 값으로 초기화
        _vector vPos = m_pTransform->Get_State(CTransform::STATE_POSITION);
        m_fLadderTopY = m_fLadderBottomY = XMVectorGetY(m_pNavigation->Compute_Height(vPos));

        // 너비 우선 탐색(BFS)으로 현재 밟고 있는 셀과 연결된 사다리 셀들을 탐색
        queue<_int> q;
        unordered_set<_int> visited;

        q.push(iCurrIdx);
        visited.insert(iCurrIdx);

        while (!q.empty())
        {
            _int iIndex = q.front();
            q.pop();

            if (iIndex < 0 || iIndex >= (_int)vecCells.size())
                continue;

            CCell* pCell = vecCells[iIndex];
            // 현재 밟고 있는 셀의 열거체 정수 값으로 LadderTop인지, LadderBottom인지 판단
            _int iFlag = pCell->Get_Flag();

            // 현재 밟고 있는 LadderTop/LadderBottom 셀의 Y 값을 계산
            // LadderTop일 경우
            if (!bFoundTop && iFlag == CCell::FLAG_LADDER_TOP)
            {
                m_fLadderTopY = m_pNavigation->Compute_Height_By_CellIndex(iIndex, vPos);
                bFoundTop = true;
            }
            // LadderBottom일 경우
            else if (!bFoundBottom && iFlag == CCell::FLAG_LADDER_BOTTOM)
            {
                m_fLadderBottomY = m_pNavigation->Compute_Height_By_CellIndex(iIndex, vPos);
                bFoundBottom = true;
            }

            if (iFlag == CCell::FLAG_LADDER_MIDDLE)
            {
                _vector vCenter = m_pNavigation->Get_CellCenter(iIndex);
                _float fDistSq = XMVectorGetX(XMVector3LengthSq(vCenter - vPlayerPos));
                
                if (fDistSq < fClosestDistSq)
                {
                    fClosestDistSq = fDistSq;
                    // 추후 플레이어가 사다리를 바라보며 타도록 사다리의 노말 벡터 저장
                    vBestMiddleNormal = m_pNavigation->Get_CellNormal(iIndex);
                }
            }

            // LadderTop, Bottom, 사다리의 노말 벡터(법선)까지 찾았다면 반복문 종료
            if (bFoundTop && bFoundBottom && fClosestDistSq < FLT_MAX)
                break;

            // 아직 못 찾았다면 인접 셀들을 더 탐색
            for (_int i = 0; i < CCell::LINE_END; ++i)
            {
                _int iNeighbor = pCell->Get_Neighbor(static_cast<CCell::LINE>(i));
                if (iNeighbor == -1 || iNeighbor >= (_int)vecCells.size())
                    continue;

                if (visited.count(iNeighbor))
                    continue;

                visited.insert(iNeighbor);
                q.push(iNeighbor);
            }
        }

        // 사다리의 노말 벡터를 반전시켜 플레이어가 바라볼 방향을 정한다
        XMStoreFloat3(&m_vClimbDir, XMVector3Normalize(XMVectorNegate(vBestMiddleNormal)));

        _vector vClimbDir = XMLoadFloat3(&m_vClimbDir);

        // 플레이어가 바라볼 방향에 맞춰 Right, Up, Look 벡터 갱신
        _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
        _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vClimbDir));
        vUp = XMVector3Normalize(XMVector3Cross(vClimbDir, vRight));

        m_pTransform->Set_State(CTransform::STATE_RIGHT, vRight);
        m_pTransform->Set_State(CTransform::STATE_UP, vUp);
        m_pTransform->Set_State(CTransform::STATE_LOOK, vClimbDir);

        // 플레이어가 LadderTop에서 탑승하는지, LadderBottom에서 탑승하는지 구분
        // 여부에 따라 탑승 애니메이션이 달라진다
        _float fY = XMVectorGetY(vPlayerPos);
        m_bClimbUp = (fabsf(fY - m_fLadderTopY) < fabsf(fY - m_fLadderBottomY));

        // 플레이어 탑승 애니메이션 재생
        m_pPlayer = dynamic_cast<CPlayer*>(m_pOwner);

        if (m_pPlayer)
        {
            CBody_Player* pBodyPlayer = dynamic_cast<CBody_Player*>(m_pPlayer->Get_PartObject(CPlayer::PART_BODY));

            if (pBodyPlayer)
            {
                m_pBodyPlayer_Model = pBodyPlayer->Get_Component<CModel>(L"Com_Model");
                NULL_CHECK(m_pBodyPlayer_Model);

                // 사다리 위에서 탑승하는 경우(LadderTop)
                if (m_bClimbUp)
                {
                    // 내려오면서 휘릭 돌아 탑승하는 모션(루프 X)
                    // 루트 모션 적용
                    m_pBodyPlayer_Model->Set_AnimationIndex(17);
                    m_bStartClimbAnimPlayed = true;
                    m_pGameInstance->Play_Sound(L"pl_gen_ladder_topmount", CHANNEL_PLAYER_LADDER, 0.1f, false);

                    // 사다리 방향 반대로 약간 밀기
                    // 제자리에서 탑승하면 사다리 표면을 타지 않고 안에 파묻히기 때문에
                    _vector vPushBack = XMLoadFloat3(&m_vClimbDir);
                    vPushBack = XMVector3Normalize(XMVectorNegate(vPushBack));
                    // 사다리의 법선 방향, 길이 1만큼 약간 밀기
                    m_pTransform->Add_Translation(vPushBack);
                }
                // 사다리 아래에서 탑승하는 경우(LadderBottom)
                else
                {
                    // 바로 기어오르기 시작하는 모션(루프 O)
                    m_pBodyPlayer_Model->Set_AnimationIndex(16, true);
                    m_bStartClimbAnimPlayed = false;
                }
            }
        }
    }
}  

void CPlayerState_Climb::Update_State(_float fTimeDelta)
{
    if (!m_pBodyPlayer_Model)
        return;

    // 위에서 탑승하는 애니메이션이 끝났으면 기는 애니메이션(루프 O)으로 전환
    if (m_bStartClimbAnimPlayed)
    {
        if (17 == m_pBodyPlayer_Model->Get_CurrentAnimationIndex() && m_pBodyPlayer_Model->Is_AnimationFinished())
        {
            m_pBodyPlayer_Model->Set_AnimationIndex(16, true);
            m_bStartClimbAnimPlayed = false;
        }
    }

    _bool bMovingUp = m_pGameInstance->Key_Pressing(DIK_W);     // 위로 이동
    _bool bMovingDown = m_pGameInstance->Key_Pressing(DIK_S);   // 아래로 이동

    // 키 입력도 없고 위에서 탑승하는 중도 아니라면, 애니메이션 일시정지
    m_pBodyPlayer_Model->Pause_Animation(!(bMovingUp || bMovingDown || m_bStartClimbAnimPlayed));

    // 키 입력으로 움직이는 중에는 사운드 재생, 멈추면 정지
    if (bMovingUp || bMovingDown)
    {
        _bool bPlaying = false;

        if (!m_pGameInstance->Is_Sound_Playing(CHANNEL_PLAYER_LADDER, &bPlaying) || !bPlaying)
        {
            m_pGameInstance->Play_Sound(L"pl_gen_ladder_climb", CHANNEL_PLAYER_LADDER, 0.1f);
            m_pGameInstance->Set_ChannelPitch(CHANNEL_PLAYER_LADDER, 0.5f);
        }
    }
    else
    {
        m_pGameInstance->Stop_Sound(CHANNEL_PLAYER_LADDER);
    }

    // W: 위로 이동
    if (bMovingUp)
    {
        m_pTransform->Climb_MoveY(m_fClimbSpeed * fTimeDelta);

        _float fY = XMVectorGetY(m_pTransform->Get_State(CTransform::STATE_POSITION));

        // 올라가다가 사다리 맨 위쪽에 도달하면 탑승 종료
        if (fY >= m_fLadderTopY)
        {
            _vector vFixed = m_pTransform->Get_State(CTransform::STATE_POSITION);
            vFixed = XMVectorSetY(vFixed, m_fLadderTopY);
            m_pTransform->Set_State(CTransform::STATE_POSITION, vFixed);

            m_pGameInstance->Play_Sound(L"pl_gen_ladder_topdismount", CHANNEL_PLAYER_LADDER, 0.3f, false);

            m_pFSM->Change_CurrentState(L"PlayerState_Idle");
            return;
        }
    }

    // S: 아래로 이동
    else if (bMovingDown)
    {
        m_pTransform->Climb_MoveY(-m_fClimbSpeed * fTimeDelta);

        _float fY = XMVectorGetY(m_pTransform->Get_State(CTransform::STATE_POSITION));

        // 내려가다가 사다리 맨 아래쪽에 도달하면 탑승 종료
        if (fY <= m_fLadderBottomY)
        {
            _vector vFixed = m_pTransform->Get_State(CTransform::STATE_POSITION);
            vFixed = XMVectorSetY(vFixed, m_fLadderBottomY);
            m_pTransform->Set_State(CTransform::STATE_POSITION, vFixed);

            m_pGameInstance->Play_Sound(L"pl_gen_ladder_topdismount", CHANNEL_PLAYER_LADDER, 0.3f, false);

            m_pFSM->Change_CurrentState(L"PlayerState_Idle");
            return;

        }
    }
}

void CPlayerState_Climb::LateUpdate_State(_float fTimeDelta)
{
}

void CPlayerState_Climb::Exit_State()
{
    m_vClimbDir = _float3(0.f, 0.f, 0.f);
    m_bStartClimbAnimPlayed = false;
    if(m_pBodyPlayer_Model)
        m_pBodyPlayer_Model->Pause_Animation(false);

    m_pGameInstance->Stop_Sound(CHANNEL_PLAYER_LADDER);
}

CPlayerState_Climb* CPlayerState_Climb::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CGameObject* pOwner)
{
    CPlayerState_Climb* pPlayerState = new CPlayerState_Climb(pDevice, pContext, pOwner);

    if (!pPlayerState)
    {
        Safe_Release(pPlayerState);
        MSG_BOX("CPlayerState_Climb Create Failed");
        return nullptr;
    }

    return pPlayerState;
}

void CPlayerState_Climb::Free()
{
    __super::Free();
}