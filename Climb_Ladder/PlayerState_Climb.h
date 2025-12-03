#pragma once
#include "Client_Defines.h"
#include "State.h"

BEGIN(Engine)
class CFSM;
class CTransform;
class CNavigation;
class CModel;
END

BEGIN(Client)

// 사다리 시작 셀을 밟은 채로 스페이스바를 눌러서 사다리에 올라타게 되는 상태
class CPlayerState_Climb final : public CState
{
private:
	CPlayerState_Climb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CGameObject* pOwner);
	CPlayerState_Climb(const CState& Prototype);
	virtual ~CPlayerState_Climb() = default;

public:
	virtual void Enter_State() override;
	virtual void Update_State(_float fTimeDelta) override;
	virtual void LateUpdate_State(_float fTimeDelta) override;
	virtual void Exit_State() override;

private:
	CFSM*			m_pFSM = { nullptr };
	CTransform*		m_pTransform = { nullptr };
	CModel*			m_pBodyPlayer_Model = { nullptr };
	CNavigation*	m_pNavigation = { nullptr };

	class CPlayer*	m_pPlayer = { nullptr };

	// 사다리의 가장 위·아래 Y 값
	_float			m_fLadderTopY = {};
	_float			m_fLadderBottomY = {};
	// 사다리에서 움직이는 속도
	_float			m_fClimbSpeed = { 5.f };
	// 사다리 진행 방향
	_float3			m_vClimbDir = {};
	// true: 올라가기 false: 내려가기
	_bool			m_bClimbUp = { true };
	// 사다리에 탑승하는 애니메이션이 재생됐는지 여부
	_bool			m_bStartClimbAnimPlayed = { false };

public:
	static CPlayerState_Climb* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CGameObject* pOwner);
	virtual void Free() override;
};

END