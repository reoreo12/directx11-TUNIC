#include "Collision_Manager.h"
#include "GameInstance.h"
#include "PartObject.h"

CCollision_Manager::CCollision_Manager()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CCollision_Manager::Initialize()
{
	return S_OK;
}

HRESULT CCollision_Manager::Add_Collider(COLLIDERGROUP eGroup, CCollider* pCollider, CGameObject* pOwner)
{
	if (eGroup >= CG_END || !pCollider)
		return E_FAIL;

	// 중복된 콜라이더가 없다면 pCollider를 원하는 그룹에 넣어준다
	for (auto& ColliderData : m_CollidersData[eGroup])
	{
		if (ColliderData.pCollider == pCollider)
			return E_FAIL;
	}

    ColliderData data;
    data.pCollider = pCollider;

    CPartObject* pPartObj = dynamic_cast<CPartObject*>(pOwner);
    
    if (pPartObj)
    {
        // PartObject의 콜라이더라면 ContainerObject에게 영향을 받은 최종 월드 행렬의 주소를 저장
        data.pWorldMatrix = pPartObj->Get_CombinedWorldMatrix();
    }
    else
    {
        // PartObject의 콜라이더가 아니라면 Transform 컴포넌트의 월드 행렬 주소를 저장
        CTransform* pTransform = pOwner->Get_Component<CTransform>(L"Com_Transform");
        
        if (pTransform)
            data.pWorldMatrix = pTransform->Get_WorldMatrix_Ptr();
        else
        {   // Transform 컴포넌트를 찾지 못 했다면 항등 행렬 저장
            static _float4x4 pIdentityMatrix;
            XMStoreFloat4x4(&pIdentityMatrix, XMMatrixIdentity());
            data.pWorldMatrix = &pIdentityMatrix;
        }
    }

    m_CollidersData[eGroup].push_back(data);

	return S_OK;
}

void CCollision_Manager::Priority_Update(_float fTimeDelta)
{
    // nullptr이거나 주인이 죽은 콜라이더를 삭제
    for (size_t i = 0; i < CG_END; i++)
    {
        auto& CollidersData = m_CollidersData[i];

        CollidersData.erase(
            remove_if(CollidersData.begin(), CollidersData.end(),
                [](const ColliderData& data)
                {
                    return data.pCollider == nullptr
                        || data.pWorldMatrix == nullptr
                        || data.pCollider->Get_Owner() == nullptr
                        || data.pCollider->Get_Owner()->Is_Dead();
                }),
            CollidersData.end());

        for (auto& data : CollidersData)
        {
            data.pCollider->Reset_Colliding(false);
        }
    }
}

void CCollision_Manager::Update(_float fTimeDelta)
{
    for (size_t i = 0; i < CG_END; i++)
    {
        for (auto& ColliderData : m_CollidersData[i])
        {
            if (ColliderData.pCollider && ColliderData.pWorldMatrix)
                ColliderData.pCollider->Update(XMLoadFloat4x4(ColliderData.pWorldMatrix));
        }
    }
}
 
void CCollision_Manager::Late_Update(_float fTimeDelta)
{
    set<pair<CCollider*, CCollider*>> checkedCollisions;

	// 충돌 검사
    for (size_t i = 0; i < CG_END; ++i)
    {
        for (size_t j = i; j < CG_END; ++j)
        {
            // 같은 그룹끼리는 충돌 검사 스킵
            if (i == j)
                continue;

            for (auto& pCollidersDataA : m_CollidersData[i])
            {
                if (!pCollidersDataA.pCollider->Is_Active())
                    continue;

                for (auto& pCollidersDataB : m_CollidersData[j])
                {
                    if (!pCollidersDataB.pCollider->Is_Active())
                        continue;

                    // 같은 콜라이더면 검사 스킵
                    if (pCollidersDataA.pCollider == pCollidersDataB.pCollider)
                        continue;

                    pCollidersDataA.pCollider->Update_CollisionState(pCollidersDataB.pCollider);
                    pCollidersDataB.pCollider->Update_CollisionState(pCollidersDataA.pCollider);
                }
            }
        }

        // 디버그 모드에서 콜라이더를 렌더링하기 위해 렌더러에 추가
#ifdef _DEBUG
        for (auto& pCollidersData : m_CollidersData[i])
        {
            m_pGameInstance->Add_Renderer_DebugComponent(pCollidersData.pCollider);
        }
#endif
    }
}
  
 CCollider* CCollision_Manager::Find_Collider(COLLIDERGROUP eGroup, CCollider* pCollider)
 {
     if (eGroup >= CG_END || !pCollider)
         return nullptr;

     for (auto& ColliderData : m_CollidersData[eGroup])
     {
         if (ColliderData.pCollider == pCollider)
             return ColliderData.pCollider;
     }

     return nullptr;
 }
 
 HRESULT CCollision_Manager::Remove_Collider(COLLIDERGROUP eGroup, CCollider* pCollider)
 {
     if (eGroup >= CG_END || !pCollider)
         return E_FAIL;

     auto& CollidersData = m_CollidersData[eGroup];

     CollidersData.erase(
         remove_if(CollidersData.begin(), CollidersData.end(),
             [pCollider](const ColliderData& data)
             {
                 return data.pCollider == pCollider;
             }),
         CollidersData.end());

     return S_OK;
 }

 void CCollision_Manager::Clear(_uint iLevelIndex)
 {
     _uint iNextLevelIndex = m_pGameInstance->Get_NextLevelIndex();

     for (_uint i = 0; i < CG_END; ++i)
     {
         auto& vecData = m_CollidersData[i];

         vecData.erase(
             remove_if(vecData.begin(), vecData.end(),
                 [=](const COLLIDERDATA& data)
                 {
                     if (data.pCollider == nullptr)
                         return true;

                     CGameObject* pOwner = data.pCollider->Get_Owner();
                     if (pOwner == nullptr)
                         return true;

                     _uint iObjLevelIndex = pOwner->Get_ObjLevelIndex();

                     return !(iObjLevelIndex == 0 || iObjLevelIndex == iNextLevelIndex);
                 }),
             vecData.end());
     }
 }
 
 CCollision_Manager* CCollision_Manager::Create()
 {
 	CCollision_Manager* pInstance = new CCollision_Manager();
 
 	if (FAILED(pInstance->Initialize()))
 	{
 		MSG_BOX("Failed to Created : CCollision_Manager");
 		Safe_Release(pInstance);
 	}
 
 	return pInstance;
 }
 
 void CCollision_Manager::Free()
 {
 	__super::Free();
 
 	Safe_Release(m_pGameInstance);
 }
 