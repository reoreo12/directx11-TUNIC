#pragma once
#include "Base.h"
#include "Collider.h"

BEGIN(Engine)
// 객체의 콜라이더 컴포넌트를 그룹으로 관리하는 매니저 클래스
class CCollision_Manager final : public CBase
{
public:
	// 콜라이더 그룹 열거체
	// 플레이어, 몬스터, 환경 오브젝트(예: 상자, 세이브 석상 등)
	enum COLLIDERGROUP { CG_PLAYER, CG_MONSTER, CG_OBJECT, CG_END };

public:
	typedef struct ColliderData
	{
		class CCollider* pCollider = { nullptr };
		const _float4x4* pWorldMatrix = { nullptr };
	} COLLIDERDATA;

private:
	CCollision_Manager();
	virtual ~CCollision_Manager() = default;

public:
	// 콜라이더를 그룹에 담아주는 함수
	HRESULT				Add_Collider(COLLIDERGROUP eGroup, class CCollider* pCollider, class CGameObject* pOwner);
	// 중복 검사용 함수
	class CCollider*	Find_Collider(COLLIDERGROUP eGroup, class CCollider* pCollider);
	// 콜라이더 삭제 함수
	HRESULT				Remove_Collider(COLLIDERGROUP eGroup, class CCollider* pCollider);
	// 콜라이더들을 전부 정리하는 함수
	void				Clear(_uint iLevelIndex);

public:
	HRESULT				Initialize();
	void				Priority_Update(_float fTimeDelta);
	void				Update(_float fTimeDelta);
	void				Late_Update(_float fTimeDelta);

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	vector<COLLIDERDATA> m_CollidersData[CG_END];

public:
	static CCollision_Manager* Create();
	virtual void Free() override;
};

END