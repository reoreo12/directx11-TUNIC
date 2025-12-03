#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CCollider final : public CComponent
{
public:
	// 콜라이더 타입 열거체(원, AABB 박스, OBB 박스)
	enum TYPE { TYPE_SPHERE, TYPE_AABB, TYPE_OBB, TYPE_END };

private:
	CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCollider(const CCollider& Prototype);
	virtual ~CCollider() = default;

public:
	// 충돌 상태 초기화 함수
	void					Reset_Colliding(_bool bIsColl) { m_bIsColl = bIsColl; }
	// 충돌 여부 확인 함수
	_bool					Is_Colliding() { return m_bIsColl; }
	// 콜라이더 타입(예: TYPE_SHPERE) 확인 함수
	TYPE					Get_ColliderType() { return m_eColliderType; }
	// 충돌 중인 상대 콜라이더들 함순
	const set<CCollider*>&	Get_CollidingSet() const { return m_CollidingWith; }
	class CBounding*		Get_Bounding() { return m_pBounding; }

public:
	// 충돌 상태 갱신 함수
	void					Update_CollisionState(CCollider* pOther);
	// 충돌 대상 목록 초기화 함수
	void					Clear_CollisionState() { m_CollidingWith.clear(); }

public:
	virtual HRESULT			Initialize_Prototype(TYPE eColliderType);
	virtual HRESULT			Initialize(void* pArg) override;
	void					Update(_fmatrix WorldMatrix);
	_bool					Intersect(CCollider* pTargetCollider);
	void					Reset() { m_bIsColl = false; }


#ifdef _DEBUG
	virtual HRESULT			Render();
#endif

private:
	TYPE									m_eColliderType = { TYPE_END };
	// DirectXCollision의 각종 Bounding 구조체와 함수를 사용하기 위한 클래스
	class CBounding*						m_pBounding = { nullptr };

	// 충돌 여부 변수
	_bool									m_bIsColl = { false };
	// 충돌 대상 목록 변순
	set<CCollider*>							m_CollidingWith;

	// 디버그 모드 콜라이더 렌더링용 변수
#ifdef _DEBUG
private:
	PrimitiveBatch<VertexPositionColor>*	m_pBatch = { nullptr };
	BasicEffect*							m_pEffect = { nullptr };
	ID3D11InputLayout*						m_pInputLayout = { nullptr };
#endif

public:
	static CCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eColliderType);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END