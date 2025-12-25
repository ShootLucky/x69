// Chams.h
#pragma once

#include "../src/SDK/SDK.h"

class CMaterials
{
	void Initialize();

	std::map<C_BaseEntity*, int> m_mapDrawnEntities = {};
	bool m_bRendering = false;
	bool m_bRenderingOriginalMat = false;
	bool m_bCleaningUp = false;

	void DrawEntity(C_BaseEntity* pEntity);
	void RunLagRecords();

public:
	IMaterial* m_pFlat = nullptr;
	IMaterial* m_pFlatIgnoreZ = nullptr;
	IMaterial* m_pShaded = nullptr;
	IMaterial* m_pShadedIgnoreZ = nullptr;
	IMaterial* m_pGlossy = nullptr;
	IMaterial* m_pGlossyIgnoreZ = nullptr;
	IMaterial* m_pGlow = nullptr;
	IMaterial* m_pGlowIgnoreZ = nullptr;
	IMaterialVar* m_pGlowEnvmapTint = nullptr;
	IMaterialVar* m_pGlowIgnoreZEnvmapTint = nullptr;
	IMaterialVar* m_pGlowSelfillumTint = nullptr;
	IMaterialVar* m_pGlowIgnoreZSelfillumTint = nullptr;
	IMaterial* m_pPlastic = nullptr;
	IMaterial* m_pPlasticIgnoreZ = nullptr;
	IMaterial* m_pFlatNoInvis = nullptr;
	IMaterial* m_pShadedNoInvis = nullptr;
	IMaterial* m_pFresnel = nullptr;
	IMaterial* m_pFresnelIgnoreZ = nullptr;
	IMaterial* m_pOverlay = nullptr;
	IMaterial* m_pOverlayIgnoreZ = nullptr;
	IMaterial* m_pKSOverlay = nullptr;
	IMaterial* m_pKSOverlayIgnoreZ = nullptr;
	IMaterial* m_pEsoOverlay = nullptr;
	IMaterial* m_pEsoOverlayIgnoreZ = nullptr;
	IMaterial* m_pFlatOverlay = nullptr;
	IMaterial* m_pFlatOverlayIgnoreZ = nullptr;

	void Run();
	void CleanUp();

	IMaterial* GetMaterial(int nIndex, bool ignorez);

	bool HasDrawn(C_BaseEntity* pEntity)
	{
		return m_mapDrawnEntities.contains(pEntity) && m_mapDrawnEntities[pEntity] > 0;
	}

	bool IsRendering()
	{
		return m_bRendering;
	}

	bool IsRenderingOriginalMat()
	{
		return m_bRenderingOriginalMat;
	}

	bool IsUsedMaterial(const IMaterial* pMaterial)
	{
		return pMaterial == m_pFlat
			|| pMaterial == m_pFlatIgnoreZ
			|| pMaterial == m_pShaded
			|| pMaterial == m_pShadedIgnoreZ
			|| pMaterial == m_pGlossy
			|| pMaterial == m_pGlossyIgnoreZ
			|| pMaterial == m_pGlow
			|| pMaterial == m_pGlowIgnoreZ
			|| pMaterial == m_pPlastic
			|| pMaterial == m_pPlasticIgnoreZ
			|| pMaterial == m_pFlatNoInvis
			|| pMaterial == m_pShadedNoInvis
			|| pMaterial == m_pFresnel
			|| pMaterial == m_pFresnelIgnoreZ
			|| pMaterial == m_pOverlay
			|| pMaterial == m_pOverlayIgnoreZ
			|| pMaterial == m_pKSOverlay
			|| pMaterial == m_pKSOverlayIgnoreZ
			|| pMaterial == m_pEsoOverlay
			|| pMaterial == m_pEsoOverlayIgnoreZ
			|| pMaterial == m_pFlatOverlay
			|| pMaterial == m_pFlatOverlayIgnoreZ;
	}

	bool IsCleaningUp() { return m_bCleaningUp; }
};

MAKE_SINGLETON_SCOPED(CMaterials, Materials, F);