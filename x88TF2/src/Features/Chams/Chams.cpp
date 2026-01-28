#include "Chams.h"
#include "CFG.h"
#include "../VisualUtils/VisualUtils.h"
#include "../LagRecords/Backtrack.h"

void SetModelStencilForOutlines(C_BaseEntity* pEntity)
{
    IMatRenderContext* pRenderContext = I::MaterialSystem->GetRenderContext();
    if (!pRenderContext)
        return;
    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal)
        return;
    auto IsEntGoingToBeGlowed = [&]()
        {
            if (pEntity->GetClassId() == ETFClassIds::CTFPlayer)
            {
                if (!CFG::Outlines_Players_Active)
                    return false;
                const auto pPlayer = pEntity->As<C_TFPlayer>();
                const bool bIsLocal = pPlayer == pLocal;
                const bool bIsFriend = pPlayer->IsPlayerOnSteamFriendsList();
                if (CFG::Outlines_Players_Ignore_Local && bIsLocal)
                    return false;
                if (CFG::Outlines_Players_Ignore_Friends && bIsFriend)
                    return false;
                if (!bIsLocal && !bIsFriend)
                {
                    if (CFG::Outlines_Players_Ignore_Teammates && pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
                    {
                        if (CFG::Outlines_Players_Show_Teammate_Medics)
                        {
                            if (pPlayer->m_iClass() != TF_CLASS_MEDIC)
                                return false;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    if (CFG::Outlines_Players_Ignore_Enemies && pPlayer->m_iTeamNum() != pLocal->m_iTeamNum())
                        return false;
                }
            }
            if (pEntity->GetClassId() == ETFClassIds::CObjectSentrygun
                || pEntity->GetClassId() == ETFClassIds::CObjectDispenser
                || pEntity->GetClassId() == ETFClassIds::CObjectTeleporter)
            {
                if (!CFG::Outlines_Buildings_Active)
                    return false;
                auto pBuilding = pEntity->As<C_BaseObject>();
                if (pBuilding->m_bPlacing())
                    return false;
                bool bIsLocal = F::VisualUtils->IsEntityOwnedBy(pBuilding, pLocal);
                if (CFG::Outlines_Buildings_Ignore_Local && bIsLocal)
                    return false;
                if (!bIsLocal)
                {
                    if (CFG::Outlines_Buildings_Ignore_Teammates && pBuilding->m_iTeamNum() == pLocal->m_iTeamNum())
                    {
                        if (CFG::Outlines_Buildings_Show_Teammate_Dispensers)
                        {
                            if (pBuilding->GetClassId() != ETFClassIds::CObjectDispenser)
                                return false;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    if (CFG::Outlines_Buildings_Ignore_Enemies && pBuilding->m_iTeamNum() != pLocal->m_iTeamNum())
                        return false;
                }
            }
            //fuck rest
            return true;
        };
    if (!IsEntGoingToBeGlowed())
    {
        pRenderContext->SetStencilEnable(false);
    }
    else
    {
        ShaderStencilState_t state = {};
        state.m_bEnable = true;
        state.m_nReferenceValue = 1;
        state.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
        state.m_PassOp = STENCILOPERATION_REPLACE;
        state.m_FailOp = STENCILOPERATION_KEEP;
        state.m_ZFailOp = STENCILOPERATION_REPLACE;
        state.SetStencilState(pRenderContext);
    }
}

void CMaterials::Initialize()
{
    static ConVar* mat_hdr_level = I::CVar->FindVar("mat_hdr_level");

    // ===== FLAT =====
    if (!m_pFlat)
    {
        KeyValues* kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFlat = I::MaterialSystem->CreateMaterial("material_flat", kv);
    }

    if (!m_pFlatIgnoreZ)
    {
        KeyValues* kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFlatIgnoreZ = I::MaterialSystem->CreateMaterial("material_flat_ignorez", kv);
    }

    // ===== SHADED =====
    if (!m_pShaded)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "vgui/white_additive");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[-0.25 1 1]");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pShaded = I::MaterialSystem->CreateMaterial("material_shaded", kv);
    }

    if (!m_pShadedIgnoreZ)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "vgui/white_additive");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[-0.25 1 1]");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pShadedIgnoreZ = I::MaterialSystem->CreateMaterial("material_shaded_ignorez", kv);
    }

    // ===== GLOSSY =====
    if (!m_pGlossy)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$envmap", "cubemaps/cubemap_sheen002");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 1 2]");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[0 0.1 1]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pGlossy = I::MaterialSystem->CreateMaterial("material_glossy", kv);
    }

    if (!m_pGlossyIgnoreZ)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$envmap", "cubemaps/cubemap_sheen002");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 1 2]");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[0 0.1 1]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pGlossyIgnoreZ = I::MaterialSystem->CreateMaterial("material_glossy_ignorez", kv);
    }

    // ===== GLOW =====
    if (!m_pGlow)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 0.05 0.1]");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[0.5 0.5 0]");
        kv->SetString("$envmaptint", "[0 1 0]");
        kv->SetString("$selfillumtint", "[0 0 0]");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pGlow = I::MaterialSystem->CreateMaterial("material_glow", kv);
        m_pGlowEnvmapTint = m_pGlow->FindVar("$envmaptint", nullptr);
        m_pGlowSelfillumTint = m_pGlow->FindVar("$selfillumtint", nullptr);
    }

    if (!m_pGlowIgnoreZ)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 0.05 0.1]");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[0.5 0.5 0]");
        kv->SetString("$envmaptint", "[0 1 0]");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pGlowIgnoreZ = I::MaterialSystem->CreateMaterial("material_glow_ignorez", kv);
        m_pGlowIgnoreZEnvmapTint = m_pGlowIgnoreZ->FindVar("$envmaptint", nullptr);
        m_pGlowIgnoreZSelfillumTint = m_pGlowIgnoreZ->FindVar("$selfillumtint", nullptr);
    }

    // ===== PLASTIC =====
    if (!m_pPlastic)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$phong", "1");
        kv->SetString("$phongexponent", "15");
        kv->SetString("$phongboost", "3");
        kv->SetString("$phongfresnelranges", "[0 0.5 1]");
        kv->SetString("$rimlight", "1");
        kv->SetString("$rimlightexponent", "2");
        kv->SetString("$rimlightboost", "1");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pPlastic = I::MaterialSystem->CreateMaterial("material_plastic", kv);
    }

    if (!m_pPlasticIgnoreZ)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$phong", "1");
        kv->SetString("$phongexponent", "15");
        kv->SetString("$phongboost", "3");
        kv->SetString("$phongfresnelranges", "[0 0.5 1]");
        kv->SetString("$rimlight", "1");
        kv->SetString("$rimlightexponent", "2");
        kv->SetString("$rimlightboost", "1");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pPlasticIgnoreZ = I::MaterialSystem->CreateMaterial("material_plastic_ignorez", kv);
    }

    // ===== FRESNEL =====
    if (!m_pFresnel)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumfresnel", "1");
        kv->SetString("$selfillumfresnelminmaxexp", "[0.3 0.8 1.5]");  // ← MUDADO: mais brilho
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$envmaptint", "[1 1 1]");
        kv->SetString("$phong", "1");
        kv->SetString("$phongexponent", "25");          // ← ADICIONADO: brilho especular
        kv->SetString("$phongboost", "2");              // ← ADICIONADO: intensidade
        kv->SetString("$phongfresnelranges", "[0 0.5 1]");
        kv->SetString("$rimlight", "1");                // ← ADICIONADO: luz nas bordas
        kv->SetString("$rimlightexponent", "3");        // ← ADICIONADO
        kv->SetString("$rimlightboost", "1.5");         // ← ADICIONADO
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFresnel = I::MaterialSystem->CreateMaterial("material_fresnel", kv);
    }

    if (!m_pFresnelIgnoreZ)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumfresnel", "1");
        kv->SetString("$selfillumfresnelminmaxexp", "[0.3 0.8 1.5]");  // ← MUDADO
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$envmaptint", "[1 1 1]");
        kv->SetString("$phong", "1");
        kv->SetString("$phongexponent", "25");          // ← ADICIONADO
        kv->SetString("$phongboost", "2");              // ← ADICIONADO
        kv->SetString("$phongfresnelranges", "[0 0.5 1]");
        kv->SetString("$rimlight", "1");                // ← ADICIONADO
        kv->SetString("$rimlightexponent", "3");        // ← ADICIONADO
        kv->SetString("$rimlightboost", "1.5");         // ← ADICIONADO
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFresnelIgnoreZ = I::MaterialSystem->CreateMaterial("material_fresnel_ignorez", kv);
    }

    // ===== OVERLAY =====
    if (!m_pOverlay)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$envmaptint", "[1 1 1]");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 0 1.5]");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pOverlay = I::MaterialSystem->CreateMaterial("material_overlay", kv);
    }

    if (!m_pOverlayIgnoreZ)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$envmaptint", "[1 1 1]");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 0 1.5]");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pOverlayIgnoreZ = I::MaterialSystem->CreateMaterial("material_overlay_ignorez", kv);
    }

    // ===== KS OVERLAY =====
    if (!m_pKSOverlay)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$sheenPassEnabled", "1");
        kv->SetString("$sheenmap", "cubemaps/cubemap_sheen001");
        kv->SetString("$sheenmapmask", "effects/AnimatedSheen/animatedsheen0");
        kv->SetString("$sheenmaptint", "[1 1 1]");
        kv->SetString("$sheenmapmaskframe", "0");
        kv->SetString("$sheenindex", "0");
        kv->SetString("$sheenmapmaskscalex", "110");
        kv->SetString("$sheenmapmaskscaley", "110");
        kv->SetString("$sheenmapmaskdirection", "2");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) {
            if (const auto animatedTexture = proxies->FindKey("AnimatedTexture", true)) {
                animatedTexture->SetString("animatedTextureVar", "$sheenmapmask");
                animatedTexture->SetString("animatedTextureFrameNumVar", "$sheenmapmaskframe");
                animatedTexture->SetString("animatedTextureFrameRate", "15");
            }
            proxies->FindKey("invis", true);
        }
        m_pKSOverlay = I::MaterialSystem->CreateMaterial("material_ksoverlay", kv);
    }

    if (!m_pKSOverlayIgnoreZ)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$sheenPassEnabled", "1");
        kv->SetString("$sheenmap", "cubemaps/cubemap_sheen001");
        kv->SetString("$sheenmapmask", "effects/AnimatedSheen/animatedsheen0");
        kv->SetString("$sheenmaptint", "[1 1 1]");
        kv->SetString("$sheenmapmaskframe", "0");
        kv->SetString("$sheenindex", "0");
        kv->SetString("$sheenmapmaskscalex", "110");
        kv->SetString("$sheenmapmaskscaley", "110");
        kv->SetString("$sheenmapmaskdirection", "2");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) {
            if (const auto animatedTexture = proxies->FindKey("AnimatedTexture", true)) {
                animatedTexture->SetString("animatedTextureVar", "$sheenmapmask");
                animatedTexture->SetString("animatedTextureFrameNumVar", "$sheenmapmaskframe");
                animatedTexture->SetString("animatedTextureFrameRate", "15");
            }
            proxies->FindKey("invis", true);
        }
        m_pKSOverlayIgnoreZ = I::MaterialSystem->CreateMaterial("material_ksoverlay_ignorez", kv);
    }

    // ===== ESO OVERLAY =====
    if (!m_pEsoOverlay)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 1.499 1.5]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pEsoOverlay = I::MaterialSystem->CreateMaterial("material_esooverlay", kv);
    }

    if (!m_pEsoOverlayIgnoreZ)
    {
        KeyValues* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 1.499 1.5]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pEsoOverlayIgnoreZ = I::MaterialSystem->CreateMaterial("material_esooverlay_ignorez", kv);
    }

    // ===== FLAT OVERLAY =====
    if (!m_pFlatOverlay)
    {
        KeyValues* kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFlatOverlay = I::MaterialSystem->CreateMaterial("material_flatoverlay", kv);
    }

    if (!m_pFlatOverlayIgnoreZ)
    {
        KeyValues* kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFlatOverlayIgnoreZ = I::MaterialSystem->CreateMaterial("material_flatoverlay_ignorez", kv);
    }
}

void CMaterials::DrawEntity(C_BaseEntity* pEntity)
{
    if (!pEntity || pEntity->IsDormant())
        return;

    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal)
        return;

    IMatRenderContext* pRenderContext = I::MaterialSystem->GetRenderContext();
    if (!pRenderContext)
        return;

    // ============== PLAYERS ==============
    if (pEntity->GetClassId() == ETFClassIds::CTFPlayer && CFG::Materials_Players_Active)
    {
        const auto pPlayer = pEntity->As<C_TFPlayer>();
        if (!pPlayer || pPlayer->deadflag())
            return;

        // Verifica filtros
        const bool bIsLocal = pPlayer == pLocal;
        const bool bIsFriend = pPlayer->IsPlayerOnSteamFriendsList();

        if (CFG::Materials_Players_Ignore_Local && bIsLocal)
            return;
        if (CFG::Materials_Players_Ignore_Friends && bIsFriend)
            return;

        if (!bIsLocal && !bIsFriend)
        {
            if (CFG::Materials_Players_Ignore_Teammates && pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
            {
                if (CFG::Materials_Players_Show_Teammate_Medics)
                {
                    if (pPlayer->m_iClass() != TF_CLASS_MEDIC)
                        return;
                }
                else
                    return;
            }
            if (CFG::Materials_Players_Ignore_Enemies && pPlayer->m_iTeamNum() != pLocal->m_iTeamNum())
                return;
        }

        // Determina cores
        Color_t color = CFG::Color_Players_Enemies;
        Color_t overlayColor = CFG::Color_Players_Overlay_Enemies;

        if (bIsLocal)
        {
            color = CFG::Color_Players_Local;
            overlayColor = CFG::Color_Players_Overlay_Local;
        }
        else if (bIsFriend)
        {
            color = CFG::Color_Players_Friends;
            overlayColor = CFG::Color_Players_Overlay_Friends;
        }
        else if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
        {
            color = CFG::Color_Players_Teammates;
            overlayColor = CFG::Color_Players_Overlay_Teammates;
        }

        // ===== OBTÉM MATERIAIS =====
        IMaterial* pMaterial = CFG::Materials_Players_Material == 0 ?
            nullptr : GetMaterial(CFG::Materials_Players_Material, false);

        IMaterial* pOverlayMat = nullptr;
        if (CFG::Materials_Players_TwoModels > 0)
        {
            switch (CFG::Materials_Players_TwoModels)
            {
            case 1: pOverlayMat = m_pFlat; break;
            case 2: pOverlayMat = m_pShaded; break;
            case 3: pOverlayMat = m_pGlossy; break;
            case 4: pOverlayMat = m_pGlow; break;
            case 5: pOverlayMat = m_pPlastic; break;
            case 6: pOverlayMat = m_pFresnel; break;
            case 7: pOverlayMat = m_pOverlay; break;
            case 8: pOverlayMat = m_pKSOverlay; break;
            case 9: pOverlayMat = m_pEsoOverlay; break;
            case 10: pOverlayMat = m_pFlatOverlay; break;
            }
        }

        // Lambda para desenhar player e attachments
        auto DrawPlayerAndAttachments = [&]()
            {
                m_bRendering = true;

                // Desenha o player
                pPlayer->DrawModel(STUDIO_RENDER);

                // Desenha a arma ativa e seus attachments
                const auto pWeapon = pPlayer->m_hActiveWeapon().Get();
                if (pWeapon && pWeapon->ShouldDraw() && !pWeapon->IsDormant())
                {
                    // Desenha a arma principal
                    pWeapon->DrawModel(STUDIO_RENDER);

                    // Desenha os attachments da arma (seringa, flechas, etc)
                    for (C_BaseEntity* pAttach = pWeapon->FirstMoveChild();
                        pAttach;
                        pAttach = pAttach->NextMovePeer())
                    {
                        if (pAttach && pAttach->ShouldDraw() && !pAttach->IsDormant())
                        {
                            pAttach->DrawModel(STUDIO_RENDER);
                        }
                    }
                }

                m_bRendering = false;
            };

        // ===== RENDERIZAÇÃO =====

        // ===== CAMADA 1: MATERIAL BASE (VISIBLE) =====
        float base_alpha = (CFG::Materials_Players_Alpha * color.a) / 255.0f;
        I::RenderView->SetBlend(base_alpha);
        I::ModelRender->ForcedMaterialOverride(pMaterial);

        if (pMaterial && pMaterial != m_pGlow)
        {
            I::RenderView->SetColorModulation(
                ColorUtils::ToFloat(color.r),
                ColorUtils::ToFloat(color.g),
                ColorUtils::ToFloat(color.b)
            );
        }
        else if (pMaterial == m_pGlow)
        {
            m_pGlowEnvmapTint->SetVecValue(
                ColorUtils::ToFloat(color.r),
                ColorUtils::ToFloat(color.g),
                ColorUtils::ToFloat(color.b)
            );

            m_pGlowSelfillumTint->SetVecValue(
                ColorUtils::ToFloat(color.r),
                ColorUtils::ToFloat(color.g),
                ColorUtils::ToFloat(color.b)
            );
        }

        DrawPlayerAndAttachments();

        // ===== CAMADA 2: MATERIAL BASE (OCCLUDED) =====
        if (CFG::Materials_Players_IgnoreDepth)
        {
            pRenderContext->DepthRange(0.0f, 0.2f);
            DrawPlayerAndAttachments();
            pRenderContext->DepthRange(0.0f, 1.0f);
        }

        // ===== CAMADA 3: OVERLAY (VISIBLE) =====
        if (pOverlayMat)
        {
            float overlay_alpha = (CFG::Materials_Players_OverlayAlpha * overlayColor.a) / 255.0f;
            I::RenderView->SetBlend(overlay_alpha);
            I::ModelRender->ForcedMaterialOverride(pOverlayMat);

            if (pOverlayMat == m_pGlow)
            {
                if (auto pEnvmapTint = pOverlayMat->FindVar("$envmaptint", nullptr))
                {
                    pEnvmapTint->SetVecValue(
                        ColorUtils::ToFloat(overlayColor.r),
                        ColorUtils::ToFloat(overlayColor.g),
                        ColorUtils::ToFloat(overlayColor.b)
                    );
                }

                if (auto pSelfillumTint = pOverlayMat->FindVar("$selfillumtint", nullptr))
                {
                    pSelfillumTint->SetVecValue(
                        ColorUtils::ToFloat(overlayColor.r),
                        ColorUtils::ToFloat(overlayColor.g),
                        ColorUtils::ToFloat(overlayColor.b)
                    );
                }
            }
            else if (pOverlayMat == m_pKSOverlay)
            {
                if (auto pVar = pOverlayMat->FindVar("$sheenmaptint", nullptr))
                {
                    pVar->SetVecValue(
                        ColorUtils::ToFloat(overlayColor.r),
                        ColorUtils::ToFloat(overlayColor.g),
                        ColorUtils::ToFloat(overlayColor.b)
                    );
                }
            }
            else
            {
                if (auto pVar = pOverlayMat->FindVar("$envmaptint", nullptr))
                {
                    pVar->SetVecValue(
                        ColorUtils::ToFloat(overlayColor.r),
                        ColorUtils::ToFloat(overlayColor.g),
                        ColorUtils::ToFloat(overlayColor.b)
                    );
                }

                I::RenderView->SetColorModulation(
                    ColorUtils::ToFloat(overlayColor.r),
                    ColorUtils::ToFloat(overlayColor.g),
                    ColorUtils::ToFloat(overlayColor.b)
                );
            }

            DrawPlayerAndAttachments();

            // ===== CAMADA 4: OVERLAY (OCCLUDED) =====
            if (CFG::Materials_Players_IgnoreDepth)
            {
                pRenderContext->DepthRange(0.0f, 0.2f);
                DrawPlayerAndAttachments();
                pRenderContext->DepthRange(0.0f, 1.0f);
            }
        }

        m_mapDrawnEntities[pEntity]++;
    }

    // ============== BUILDINGS ==============
    else if ((pEntity->GetClassId() == ETFClassIds::CObjectSentrygun ||
        pEntity->GetClassId() == ETFClassIds::CObjectDispenser ||
        pEntity->GetClassId() == ETFClassIds::CObjectTeleporter) &&
        CFG::Materials_Buildings_Active)
    {
        const auto pBuilding = pEntity->As<C_BaseObject>();
        if (!pBuilding || pBuilding->m_bPlacing())
            return;

        // Verifica filtros
        bool bIsLocal = F::VisualUtils->IsEntityOwnedBy(pBuilding, pLocal);

        if (CFG::Materials_Buildings_Ignore_Local && bIsLocal)
            return;

        if (!bIsLocal)
        {
            if (CFG::Materials_Buildings_Ignore_Teammates && pBuilding->m_iTeamNum() == pLocal->m_iTeamNum())
            {
                if (CFG::Materials_Buildings_Show_Teammate_Dispensers)
                {
                    if (pBuilding->GetClassId() != ETFClassIds::CObjectDispenser)
                        return;
                }
                else
                    return;
            }
            if (CFG::Materials_Buildings_Ignore_Enemies && pBuilding->m_iTeamNum() != pLocal->m_iTeamNum())
                return;
        }

        // Determina cores
        Color_t color = CFG::Color_Buildings_Enemies;
        Color_t overlayColor = CFG::Color_Buildings_Overlay_Enemies;

        if (bIsLocal)
        {
            color = CFG::Color_Buildings_Local;
            overlayColor = CFG::Color_Buildings_Overlay_Local;
        }
        else if (pBuilding->m_iTeamNum() == pLocal->m_iTeamNum())
        {
            color = CFG::Color_Buildings_Teammates;
            overlayColor = CFG::Color_Buildings_Overlay_Teammates;
        }

        // ===== OBTÉM MATERIAIS =====
        IMaterial* pMaterial = CFG::Materials_Buildings_Material == 0 ?
            nullptr : GetMaterial(CFG::Materials_Buildings_Material, false);

        IMaterial* pOverlayMat = nullptr;
        if (CFG::Materials_Buildings_TwoModels > 0)
        {
            switch (CFG::Materials_Buildings_TwoModels)
            {
            case 1: pOverlayMat = m_pFlat; break;
            case 2: pOverlayMat = m_pShaded; break;
            case 3: pOverlayMat = m_pGlossy; break;
            case 4: pOverlayMat = m_pGlow; break;
            case 5: pOverlayMat = m_pPlastic; break;
            case 6: pOverlayMat = m_pFresnel; break;
            case 7: pOverlayMat = m_pOverlay; break;
            case 8: pOverlayMat = m_pKSOverlay; break;
            case 9: pOverlayMat = m_pEsoOverlay; break;
            case 10: pOverlayMat = m_pFlatOverlay; break;
            }
        }

        // Lambda para desenhar building e attachments
        auto DrawBuilding = [&]()
            {
                m_bRendering = true;
                pBuilding->DrawModel(STUDIO_RENDER);

                // Desenhar attachments de buildings
                for (C_BaseEntity* pAttach = pBuilding->FirstMoveChild();
                    pAttach;
                    pAttach = pAttach->NextMovePeer())
                {
                    if (pAttach && pAttach->ShouldDraw() && !pAttach->IsDormant())
                    {
                        pAttach->DrawModel(STUDIO_RENDER);
                    }
                }

                m_bRendering = false;
            };

        // ===== RENDERIZAÇÃO =====

        // ===== CAMADA 1: MATERIAL BASE (VISIBLE) =====
        float base_alpha = (CFG::Materials_Buildings_Alpha * color.a) / 255.0f;
        I::RenderView->SetBlend(base_alpha);
        I::ModelRender->ForcedMaterialOverride(pMaterial);

        if (pMaterial && pMaterial != m_pGlow)
        {
            I::RenderView->SetColorModulation(
                ColorUtils::ToFloat(color.r),
                ColorUtils::ToFloat(color.g),
                ColorUtils::ToFloat(color.b)
            );
        }
        else if (pMaterial == m_pGlow)
        {
            m_pGlowEnvmapTint->SetVecValue(
                ColorUtils::ToFloat(color.r),
                ColorUtils::ToFloat(color.g),
                ColorUtils::ToFloat(color.b)
            );

            m_pGlowSelfillumTint->SetVecValue(
                ColorUtils::ToFloat(color.r),
                ColorUtils::ToFloat(color.g),
                ColorUtils::ToFloat(color.b)
            );
        }

        DrawBuilding();

        // ===== CAMADA 2: MATERIAL BASE (OCCLUDED) =====
        if (CFG::Materials_Buildings_IgnoreDepth)
        {
            pRenderContext->DepthRange(0.0f, 0.2f);
            DrawBuilding();
            pRenderContext->DepthRange(0.0f, 1.0f);
        }

        // ===== CAMADA 3: OVERLAY (VISIBLE) =====
        if (pOverlayMat)
        {
            float overlay_alpha = (CFG::Materials_Buildings_OverlayAlpha * overlayColor.a) / 255.0f;
            I::RenderView->SetBlend(overlay_alpha);
            I::ModelRender->ForcedMaterialOverride(pOverlayMat);

            if (pOverlayMat == m_pGlow)
            {
                if (auto pEnvmapTint = pOverlayMat->FindVar("$envmaptint", nullptr))
                {
                    pEnvmapTint->SetVecValue(
                        ColorUtils::ToFloat(overlayColor.r),
                        ColorUtils::ToFloat(overlayColor.g),
                        ColorUtils::ToFloat(overlayColor.b)
                    );
                }

                if (auto pSelfillumTint = pOverlayMat->FindVar("$selfillumtint", nullptr))
                {
                    pSelfillumTint->SetVecValue(
                        ColorUtils::ToFloat(overlayColor.r),
                        ColorUtils::ToFloat(overlayColor.g),
                        ColorUtils::ToFloat(overlayColor.b)
                    );
                }
            }
            else if (pOverlayMat == m_pKSOverlay)
            {
                if (auto pVar = pOverlayMat->FindVar("$sheenmaptint", nullptr))
                {
                    pVar->SetVecValue(
                        ColorUtils::ToFloat(overlayColor.r),
                        ColorUtils::ToFloat(overlayColor.g),
                        ColorUtils::ToFloat(overlayColor.b)
                    );
                }
            }
            else
            {
                if (auto pVar = pOverlayMat->FindVar("$envmaptint", nullptr))
                {
                    pVar->SetVecValue(
                        ColorUtils::ToFloat(overlayColor.r),
                        ColorUtils::ToFloat(overlayColor.g),
                        ColorUtils::ToFloat(overlayColor.b)
                    );
                }

                I::RenderView->SetColorModulation(
                    ColorUtils::ToFloat(overlayColor.r),
                    ColorUtils::ToFloat(overlayColor.g),
                    ColorUtils::ToFloat(overlayColor.b)
                );
            }

            DrawBuilding();

            // ===== CAMADA 4: OVERLAY (OCCLUDED) =====
            if (CFG::Materials_Buildings_IgnoreDepth)
            {
                pRenderContext->DepthRange(0.0f, 0.2f);
                DrawBuilding();
                pRenderContext->DepthRange(0.0f, 1.0f);
            }
        }

        m_mapDrawnEntities[pEntity]++;
    }

    // Restaura estado padrão
    I::ModelRender->ForcedMaterialOverride(nullptr);
    I::RenderView->SetBlend(1.f);
    I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
    pRenderContext->DepthRange(0.f, 1.f);
}


IMaterial* CMaterials::GetMaterial(int nIndex, bool ignorez)
{
    switch (nIndex)
    {
    case 1: return ignorez ? m_pFlatIgnoreZ : m_pFlat;
    case 2: return ignorez ? m_pShadedIgnoreZ : m_pShaded;
    case 3: return ignorez ? m_pGlossyIgnoreZ : m_pGlossy;
    case 4: return ignorez ? m_pGlowIgnoreZ : m_pGlow;
    case 5: return ignorez ? m_pPlasticIgnoreZ : m_pPlastic;
    case 6: return ignorez ? m_pFresnelIgnoreZ : m_pFresnel;
    default: return nullptr;
    }
}

void CMaterials::Run()
{
    if (!CFG::Materials_Active)
        return;

    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal)
        return;

    Initialize();
    m_mapDrawnEntities.clear();

    for (int i = 1; i <= I::ClientEntityList->GetHighestEntityIndex(); i++)
    {
        auto pEntity = I::ClientEntityList->GetClientEntity(i);
        if (!pEntity)
            continue;

        DrawEntity(reinterpret_cast<C_BaseEntity*>(pEntity));  // ← CAST IMPORTANTE
    }
}

void CMaterials::RunLagRecords()
{
    return;
}

void CMaterials::CleanUp()
{
    m_bCleaningUp = true;

    // ===== FLAT =====
    if (m_pFlat)
    {
        m_pFlat->DecrementReferenceCount();
        m_pFlat->DeleteIfUnreferenced();
        m_pFlat = nullptr;
    }

    if (m_pFlatIgnoreZ)
    {
        m_pFlatIgnoreZ->DecrementReferenceCount();
        m_pFlatIgnoreZ->DeleteIfUnreferenced();
        m_pFlatIgnoreZ = nullptr;
    }

    // ===== SHADED =====
    if (m_pShaded)
    {
        m_pShaded->DecrementReferenceCount();
        m_pShaded->DeleteIfUnreferenced();
        m_pShaded = nullptr;
    }

    if (m_pShadedIgnoreZ)
    {
        m_pShadedIgnoreZ->DecrementReferenceCount();
        m_pShadedIgnoreZ->DeleteIfUnreferenced();
        m_pShadedIgnoreZ = nullptr;
    }

    // ===== GLOSSY =====
    if (m_pGlossy)
    {
        m_pGlossy->DecrementReferenceCount();
        m_pGlossy->DeleteIfUnreferenced();
        m_pGlossy = nullptr;
    }

    if (m_pGlossyIgnoreZ)
    {
        m_pGlossyIgnoreZ->DecrementReferenceCount();
        m_pGlossyIgnoreZ->DeleteIfUnreferenced();
        m_pGlossyIgnoreZ = nullptr;
    }

    // ===== GLOW =====
    if (m_pGlow)
    {
        m_pGlow->DecrementReferenceCount();
        m_pGlow->DeleteIfUnreferenced();
        m_pGlow = nullptr;
        m_pGlowEnvmapTint = nullptr;
        m_pGlowSelfillumTint = nullptr;
    }

    if (m_pGlowIgnoreZ)
    {
        m_pGlowIgnoreZ->DecrementReferenceCount();
        m_pGlowIgnoreZ->DeleteIfUnreferenced();
        m_pGlowIgnoreZ = nullptr;
        m_pGlowIgnoreZEnvmapTint = nullptr;
        m_pGlowIgnoreZSelfillumTint = nullptr;
    }

    // ===== PLASTIC =====
    if (m_pPlastic)
    {
        m_pPlastic->DecrementReferenceCount();
        m_pPlastic->DeleteIfUnreferenced();
        m_pPlastic = nullptr;
    }

    if (m_pPlasticIgnoreZ)
    {
        m_pPlasticIgnoreZ->DecrementReferenceCount();
        m_pPlasticIgnoreZ->DeleteIfUnreferenced();
        m_pPlasticIgnoreZ = nullptr;
    }

    // ===== FLAT NO INVIS =====
    if (m_pFlatNoInvis)
    {
        m_pFlatNoInvis->DecrementReferenceCount();
        m_pFlatNoInvis->DeleteIfUnreferenced();
        m_pFlatNoInvis = nullptr;
    }

    // ===== SHADED NO INVIS =====
    if (m_pShadedNoInvis)
    {
        m_pShadedNoInvis->DecrementReferenceCount();
        m_pShadedNoInvis->DeleteIfUnreferenced();
        m_pShadedNoInvis = nullptr;
    }

    // ===== FRESNEL =====
    if (m_pFresnel)
    {
        m_pFresnel->DecrementReferenceCount();
        m_pFresnel->DeleteIfUnreferenced();
        m_pFresnel = nullptr;
    }

    if (m_pFresnelIgnoreZ)
    {
        m_pFresnelIgnoreZ->DecrementReferenceCount();
        m_pFresnelIgnoreZ->DeleteIfUnreferenced();
        m_pFresnelIgnoreZ = nullptr;
    }

    // ===== OVERLAY =====
    if (m_pOverlay)
    {
        m_pOverlay->DecrementReferenceCount();
        m_pOverlay->DeleteIfUnreferenced();
        m_pOverlay = nullptr;
    }

    if (m_pOverlayIgnoreZ)
    {
        m_pOverlayIgnoreZ->DecrementReferenceCount();
        m_pOverlayIgnoreZ->DeleteIfUnreferenced();
        m_pOverlayIgnoreZ = nullptr;
    }

    // ===== KS OVERLAY =====
    if (m_pKSOverlay)
    {
        m_pKSOverlay->DecrementReferenceCount();
        m_pKSOverlay->DeleteIfUnreferenced();
        m_pKSOverlay = nullptr;
    }

    if (m_pKSOverlayIgnoreZ)
    {
        m_pKSOverlayIgnoreZ->DecrementReferenceCount();
        m_pKSOverlayIgnoreZ->DeleteIfUnreferenced();
        m_pKSOverlayIgnoreZ = nullptr;
    }

    // ===== ESO OVERLAY =====
    if (m_pEsoOverlay)
    {
        m_pEsoOverlay->DecrementReferenceCount();
        m_pEsoOverlay->DeleteIfUnreferenced();
        m_pEsoOverlay = nullptr;
    }

    if (m_pEsoOverlayIgnoreZ)
    {
        m_pEsoOverlayIgnoreZ->DecrementReferenceCount();
        m_pEsoOverlayIgnoreZ->DeleteIfUnreferenced();
        m_pEsoOverlayIgnoreZ = nullptr;
    }

    // ===== FLAT OVERLAY =====
    if (m_pFlatOverlay)
    {
        m_pFlatOverlay->DecrementReferenceCount();
        m_pFlatOverlay->DeleteIfUnreferenced();
        m_pFlatOverlay = nullptr;
    }

    if (m_pFlatOverlayIgnoreZ)
    {
        m_pFlatOverlayIgnoreZ->DecrementReferenceCount();
        m_pFlatOverlayIgnoreZ->DeleteIfUnreferenced();
        m_pFlatOverlayIgnoreZ = nullptr;
    }

    m_bCleaningUp = false;
}