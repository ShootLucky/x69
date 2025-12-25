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
    if (!m_pFlat)
    {
        auto* kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFlat = I::MaterialSystem->CreateMaterial("seo_material_flat", kv);
    }
    if (!m_pFlatIgnoreZ)
    {
        auto* kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFlatIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_flat_ignorez", kv);
    }
    if (!m_pShaded)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "vgui/white_additive");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[-0.25 1 1]");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pShaded = I::MaterialSystem->CreateMaterial("seo_material_shaded", kv);
    }
    if (!m_pShadedIgnoreZ)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "vgui/white_additive");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[-0.25 1 1]");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pShadedIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_shaded_ignorez", kv);
    }
    if (!m_pGlossy)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
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
        m_pGlossy = I::MaterialSystem->CreateMaterial("seo_material_glossy", kv);
    }
    if (!m_pGlossyIgnoreZ)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
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
        m_pGlossyIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_glossy_ignorez", kv);
    }
    if (!m_pGlow)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$addtivie", "0");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 0.05 0.1]");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[0.5 0.5 0]");
        kv->SetString("$envmaptint", "[0 1 0]");
        kv->SetString("$selfillumtint", "[0 0 0]");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pGlow = I::MaterialSystem->CreateMaterial("seo_material_glow", kv);
        m_pGlowEnvmapTint = m_pGlow->FindVar("$envmaptint", nullptr);
        m_pGlowSelfillumTint = m_pGlow->FindVar("$selfillumtint", nullptr);
    }
    if (!m_pGlowIgnoreZ)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$addtivie", "0");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 0.05 0.1]");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[0.5 0.5 0]");
        kv->SetString("$envmaptint", "[0 1 0]");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pGlowIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_glow_ignorez", kv);
        m_pGlowIgnoreZEnvmapTint = m_pGlowIgnoreZ->FindVar("$envmaptint", nullptr);
        m_pGlowIgnoreZSelfillumTint = m_pGlowIgnoreZ->FindVar("$selfillumtint", nullptr);
    }
    if (!m_pPlastic)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "models/player/shared/ice_player");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$phong", "1");
        kv->SetString("$phongexponent", "10");
        kv->SetString("$phongboost", "1");
        kv->SetString("$phongfresnelranges", "[0 0 0]");
        kv->SetString("$basemapalphaphongmask", "1");
        kv->SetString("$phongwarptexture", "models/player/shared/ice_player_warp");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pPlastic = I::MaterialSystem->CreateMaterial("seo_material_plastic", kv);
    }
    if (!m_pPlasticIgnoreZ)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "models/player/shared/ice_player");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$phong", "1");
        kv->SetString("$phongexponent", "10");
        kv->SetString("$phongboost", "1");
        kv->SetString("$phongfresnelranges", "[0 0 0]");
        kv->SetString("$basemapalphaphongmask", "1");
        kv->SetString("$phongwarptexture", "models/player/shared/ice_player_warp");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pPlasticIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_plastic_ignorez", kv);
    }
    if (!m_pFlatNoInvis)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "vgui/white_additive");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[0.4999 0.5 1]");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        m_pFlatNoInvis = I::MaterialSystem->CreateMaterial("seo_material_flat_no_invis", kv);
    }
    if (!m_pShadedNoInvis)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelMinMaxExp", "[0.1 0.5 2]");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        m_pShadedNoInvis = I::MaterialSystem->CreateMaterial("seo_material_shaded_no_invis", kv);
    }
    if (!m_pFresnel)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelminmaxexp", "[0.5 0.5 0]");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$envmaptint", "[1 1 1]");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 0.5 1]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFresnel = I::MaterialSystem->CreateMaterial("seo_material_fresnel", kv);
    }
    if (!m_pFresnelIgnoreZ)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumFresnelminmaxexp", "[0.5 0.5 0]");
        kv->SetString("$envmap", "skybox/sky_dustbowl_01");
        kv->SetString("$envmapfresnel", "1");
        kv->SetString("$envmaptint", "[1 1 1]");
        kv->SetString("$phong", "1");
        kv->SetString("$phongfresnelranges", "[0 0.5 1]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        kv->SetString("$ignorez", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFresnelIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_fresnel_ignorez", kv);
    }
    if (!m_pOverlay)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
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
        m_pOverlay = I::MaterialSystem->CreateMaterial("seo_material_overlay", kv);
    }
    if (!m_pOverlayIgnoreZ)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
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
        m_pOverlayIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_overlay_ignorez", kv);
    }
    if (!m_pKSOverlay)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");  // Adicionado para consistência com outros overlays
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$sheenPassEnabled", "1");
        kv->SetString("$sheenmap", "cubemaps/cubemap_sheen001");
        kv->SetString("$sheenmapmask", "effects/AnimatedSheen/animatedsheen0");
        kv->SetString("$sheenmaptint", "[1 1 1]");  // Reduzido para [1 1 1] para evitar overbright e problemas de cor
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
        m_pKSOverlay = I::MaterialSystem->CreateMaterial("seo_material_ksoverlay", kv);
    }
    if (!m_pKSOverlayIgnoreZ)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$bumpmap", "models/player/shared/shared_normal");  // Adicionado para consistência com outros overlays
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$sheenPassEnabled", "1");
        kv->SetString("$sheenmap", "cubemaps/cubemap_sheen001");
        kv->SetString("$sheenmapmask", "effects/AnimatedSheen/animatedsheen0");
        kv->SetString("$sheenmaptint", "[1 1 1]");  // Reduzido para [1 1 1] para evitar overbright e problemas de cor
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
        m_pKSOverlayIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_ksoverlay_ignorez", kv);
    }
    if (!m_pEsoOverlay)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
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
        m_pEsoOverlay = I::MaterialSystem->CreateMaterial("seo_material_esooverlay", kv);
    }
    if (!m_pEsoOverlayIgnoreZ)
    {
        auto* kv = new KeyValues("VertexLitGeneric");
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
        m_pEsoOverlayIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_esooverlay_ignorez", kv);
    }
    if (!m_pFlatOverlay)
    {
        auto* kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "vgui/white_additive");
        kv->SetString("$additive", "1");
        kv->SetString("$selfillum", "1");
        kv->SetString("$selfillumFresnel", "1");
        kv->SetString("$selfillumtint", "[0 0 0]");
        kv->SetString("$cloakPassEnabled", "1");
        kv->SetString("$nodecal", "1");
        kv->SetString("$model", "1");
        if (const auto proxies = kv->FindKey("Proxies", true)) { proxies->FindKey("invis", true); }
        m_pFlatOverlay = I::MaterialSystem->CreateMaterial("seo_material_flatoverlay", kv);
    }
    if (!m_pFlatOverlayIgnoreZ)
    {
        auto* kv = new KeyValues("UnlitGeneric");
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
        m_pFlatOverlayIgnoreZ = I::MaterialSystem->CreateMaterial("seo_material_flatoverlay_ignorez", kv);
    }
}

void CMaterials::DrawEntity(C_BaseEntity* pEntity)
{
    SetModelStencilForOutlines(pEntity);
    m_bRendering = true;
    const float flOldInvisibility = pEntity->GetClassId() == ETFClassIds::CTFPlayer ? pEntity->As<C_TFPlayer>()->m_flInvisibility() : -1.0f;
    if (flOldInvisibility > 0.99f)
    {
        pEntity->As<C_TFPlayer>()->m_flInvisibility() = 0.0f;
        I::RenderView->SetBlend(1.0f);
    }
    pEntity->DrawModel(STUDIO_RENDER);
    if (flOldInvisibility > 0.99f)
    {
        pEntity->As<C_TFPlayer>()->m_flInvisibility() = flOldInvisibility;
        I::RenderView->SetBlend(1.0f);
    }
    m_mapDrawnEntities[pEntity]++;
    m_bRendering = false;
}

void CMaterials::RunLagRecords()
{
    const auto pRenderContext = I::MaterialSystem->GetRenderContext();
    if (!pRenderContext || !CFG::Materials_Players_Active || CFG::Materials_Players_Ignore_LagRecords)
        return;
    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal)
        return;
    const auto pWeapon = H::Entities->GetWeapon();
    if (!pWeapon)
        return;
    const auto weaponType = H::AimUtils->GetWeaponType(pWeapon);
    if (weaponType == EWeaponType::HITSCAN)
    {
        if (!CFG::Aimbot_Active
            || !CFG::Aimbot_Hitscan_Active
            || !CFG::Aimbot_Target_Players
            || !CFG::Aimbot_Hitscan_Target_LagRecords)
            return;
    }
    else if (weaponType == EWeaponType::MELEE)
    {
        if (!CFG::Aimbot_Active
            || !CFG::Aimbot_Melee_Active
            || !CFG::Aimbot_Target_Players
            || !CFG::Aimbot_Melee_Target_LagRecords)
            return;
    }
    else
    {
        return;
    }
    m_bRenderingOriginalMat = false;
    I::RenderView->SetColorModulation(ColorUtils::ToFloat(CFG::Color_Players_LagRecords.r), ColorUtils::ToFloat(CFG::Color_Players_LagRecords.g), ColorUtils::ToFloat(CFG::Color_Players_LagRecords.b));
    I::ModelRender->ForcedMaterialOverride(CFG::Materials_Players_LagRecords_Style == 0 ? m_pFlatNoInvis : m_pShadedNoInvis);
    if (CFG::Materials_Players_No_Depth)
        pRenderContext->DepthRange(0.0f, 0.2f);
    for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
    {
        if (!pEntity)
            continue;
        const auto pPlayer = pEntity->As<C_TFPlayer>();
        if (pPlayer->deadflag())
            continue;
        int nRecords = 0;
        if (!F::LagRecords->HasRecords(pPlayer, &nRecords))
            continue;
        if (nRecords <= 0)
            continue;
        if (CFG::Materials_Players_LagRecords_Style == 0)
        {
            for (int n = 1; n < nRecords; n++)
            {
                const auto pRecord = F::LagRecords->GetRecord(pPlayer, n, true);
                if (!pRecord || !F::VisualUtils->IsOnScreenNoEntity(pLocal, pRecord->AbsOrigin) || !F::LagRecords->DiffersFromCurrent(pRecord))
                    continue;
                I::RenderView->SetBlend(Math::RemapValClamped(static_cast<float>(n), 1.0f, static_cast<float>(nRecords), 0.1f, 0.001f));
                F::LagRecordMatrixHelper->Set(pRecord);
                m_bRendering = true;
                const float flOldInvisibility = pPlayer->m_flInvisibility();
                pPlayer->m_flInvisibility() = 0.0f;
                pPlayer->DrawModel(STUDIO_RENDER | STUDIO_NOSHADOWS);
                pPlayer->m_flInvisibility() = flOldInvisibility;
                m_bRendering = false;
                F::LagRecordMatrixHelper->Restore();
            }
        }
        else
        {
            const auto pRecord = F::LagRecords->GetRecord(pPlayer, nRecords - 1, true);
            if (!pRecord || !F::VisualUtils->IsOnScreenNoEntity(pLocal, pRecord->AbsOrigin) || !F::LagRecords->DiffersFromCurrent(pRecord))
                continue;
            I::RenderView->SetBlend(1.0f);
            F::LagRecordMatrixHelper->Set(pRecord);
            m_bRendering = true;
            const float flOldInvisibility = pPlayer->m_flInvisibility();
            pPlayer->m_flInvisibility() = 0.0f;
            pPlayer->DrawModel(STUDIO_RENDER | STUDIO_NOSHADOWS);
            pPlayer->m_flInvisibility() = flOldInvisibility;
            m_bRendering = false;
            F::LagRecordMatrixHelper->Restore();
        }
    }
    I::ModelRender->ForcedMaterialOverride(nullptr);
    if (CFG::Materials_Players_No_Depth)
        pRenderContext->DepthRange(0.0f, 1.0f);
    I::RenderView->SetBlend(1.0f);
}

IMaterial* CMaterials::GetMaterial(int nIndex, bool ignorez)
{
    m_bRenderingOriginalMat = nIndex == 0 || nIndex > 10;
    switch (nIndex)
    {
    case 0: return nullptr;
    case 1: return ignorez ? m_pFlatIgnoreZ : m_pFlat;
    case 2: return ignorez ? m_pShadedIgnoreZ : m_pShaded;
    case 3: return ignorez ? m_pGlossyIgnoreZ : m_pGlossy;
    case 4: return ignorez ? m_pGlowIgnoreZ : m_pGlow;
    case 5: return ignorez ? m_pPlasticIgnoreZ : m_pPlastic;
    case 6: return ignorez ? m_pFresnelIgnoreZ : m_pFresnel;
    case 7: return ignorez ? m_pOverlayIgnoreZ : m_pOverlay;
    case 8: return ignorez ? m_pKSOverlayIgnoreZ : m_pKSOverlay;
    case 9: return ignorez ? m_pEsoOverlayIgnoreZ : m_pEsoOverlay;
    case 10: return ignorez ? m_pFlatOverlayIgnoreZ : m_pFlatOverlay;
    default: return nullptr;
    }
}

void CMaterials::Run()
{
    Initialize();
    if (!m_mapDrawnEntities.empty())
        m_mapDrawnEntities.clear();
    if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
    {
        return;
    }
    // Early exit if nothing is enabled - massive performance gain
    if (!CFG::Materials_Players_Active && !CFG::Materials_Buildings_Active && !CFG::Materials_World_Active && !CFG::Materials_Hands_Active && !CFG::Materials_Weapons_Active)
        return;
    const auto pRenderContext = I::MaterialSystem->GetRenderContext();
    if (!pRenderContext)
        return;
    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal)
        return;
    m_pGlowSelfillumTint->SetVecValue(0.03f, 0.03f, 0.03f);
    m_pGlowIgnoreZSelfillumTint->SetVecValue(0.03f, 0.03f, 0.03f);
    RunLagRecords();
    if (CFG::Materials_Players_Active)
    {
        I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
        if (CFG::Materials_Players_Alpha < 1.0f)
            I::RenderView->SetBlend(CFG::Materials_Players_Alpha);
        // Cache local team for faster comparisons
        const int nLocalTeam = pLocal->m_iTeamNum();
        for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ALL))
        {
            if (!pEntity)
                continue;
            const auto pPlayer = pEntity->As<C_TFPlayer>();
            if (pPlayer->deadflag())
                continue;
            const bool bIsLocal = pPlayer == pLocal;
            if (CFG::Materials_Players_Ignore_Local && bIsLocal)
                continue;
            // Early screen check before expensive friend check
            if (!F::VisualUtils->IsOnScreen(pLocal, pPlayer))
                continue;
            const bool bIsFriend = pPlayer->IsPlayerOnSteamFriendsList();
            if (CFG::Materials_Players_Ignore_Friends && bIsFriend)
                continue;
            if (!bIsLocal && !bIsFriend)
            {
                const int nPlayerTeam = pPlayer->m_iTeamNum();
                if (CFG::Materials_Players_Ignore_Teammates && nPlayerTeam == nLocalTeam)
                {
                    if (!CFG::Materials_Players_Show_Teammate_Medics || pPlayer->m_iClass() != TF_CLASS_MEDIC)
                        continue;
                }
                if (CFG::Materials_Players_Ignore_Enemies && nPlayerTeam != nLocalTeam)
                    continue;
            }
            Color_t entColor;
            Color_t overlayColor;
            if (bIsLocal)
            {
                entColor = CFG::Color_Players_Local;
                overlayColor = CFG::Color_Players_Overlay_Local;
            }
            else if (bIsFriend)
            {
                entColor = CFG::Color_Players_Friends;
                overlayColor = CFG::Color_Players_Overlay_Friends;
            }
            else if (pPlayer->m_iTeamNum() == nLocalTeam)
            {
                entColor = CFG::Color_Players_Teammates;
                overlayColor = CFG::Color_Players_Overlay_Teammates;
            }
            else
            {
                entColor = CFG::Color_Players_Enemies;
                overlayColor = CFG::Color_Players_Overlay_Enemies;
            }
            IMaterial* pMaterial = CFG::Materials_Players_Material == 0 ? nullptr : GetMaterial(CFG::Materials_Players_Material, false);
            IMaterial* pMaterialHidden = (CFG::Materials_Players_HiddenMaterial && CFG::Materials_Players_Material != 0) ? GetMaterial(CFG::Materials_Players_Material, true) : nullptr;
            int overlayType = CFG::Materials_Players_TwoModels;
            IMaterial* pOverlay = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, false);
            IMaterial* pOverlayHidden = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, true);
            auto DrawPlayerAndAttachments = [&]()
                {
                    DrawEntity(pPlayer);
                    auto pAttach = pPlayer->FirstMoveChild();
                    for (int n = 0; n < 32; n++)
                    {
                        if (!pAttach)
                            break;
                        if (pAttach->ShouldDraw())
                            DrawEntity(pAttach);
                        pAttach = pAttach->NextMovePeer();
                    }
                };
            if (pMaterialHidden)
            {
                I::ModelRender->ForcedMaterialOverride(pMaterialHidden);
                if (pMaterialHidden != m_pGlow && pMaterialHidden != m_pGlowIgnoreZ)
                    I::RenderView->SetColorModulation(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
                else if (pMaterialHidden == m_pGlowIgnoreZ)
                    m_pGlowIgnoreZEnvmapTint->SetVecValue(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
                DrawPlayerAndAttachments();
            }
            if (CFG::Materials_Players_No_Depth)
                pRenderContext->DepthRange(0.0f, 0.2f);
            I::ModelRender->ForcedMaterialOverride(pMaterial);
            if (pMaterial && pMaterial != m_pGlow)
                I::RenderView->SetColorModulation(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
            else if (pMaterial == m_pGlow)
                m_pGlowEnvmapTint->SetVecValue(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
            DrawPlayerAndAttachments();
            if (overlayType > 0)
            {
                I::ModelRender->ForcedMaterialOverride(pOverlayHidden);
                I::RenderView->SetColorModulation(ColorUtils::ToFloat(overlayColor.r), ColorUtils::ToFloat(overlayColor.g), ColorUtils::ToFloat(overlayColor.b));
                DrawPlayerAndAttachments();
                I::ModelRender->ForcedMaterialOverride(pOverlay);
                I::RenderView->SetColorModulation(ColorUtils::ToFloat(overlayColor.r), ColorUtils::ToFloat(overlayColor.g), ColorUtils::ToFloat(overlayColor.b));
                DrawPlayerAndAttachments();
            }
            if (CFG::Materials_Players_No_Depth)
                pRenderContext->DepthRange(0.0f, 1.0f);
        }
        I::ModelRender->ForcedMaterialOverride(nullptr);
        I::RenderView->SetBlend(1.0f);
    }
    if (CFG::Materials_Buildings_Active)
    {
        I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
        if (CFG::Materials_Buildings_Alpha < 1.0f)
            I::RenderView->SetBlend(CFG::Materials_Buildings_Alpha);
        for (const auto pEntity : H::Entities->GetGroup(EEntGroup::BUILDINGS_ALL))
        {
            if (!pEntity)
                continue;
            const auto pBuilding = pEntity->As<C_BaseObject>();
            if (pBuilding->m_bPlacing())
                continue;
            const bool bIsLocal = F::VisualUtils->IsEntityOwnedBy(pBuilding, pLocal);
            if (CFG::Materials_Buildings_Ignore_Local && bIsLocal)
                continue;
            if (!bIsLocal)
            {
                if (CFG::Materials_Buildings_Ignore_Teammates && pBuilding->m_iTeamNum() == pLocal->m_iTeamNum())
                {
                    if (CFG::Materials_Buildings_Show_Teammate_Dispensers)
                    {
                        if (pBuilding->GetClassId() != ETFClassIds::CObjectDispenser)
                            continue;
                    }
                    else
                    {
                        continue;
                    }
                }
                if (CFG::Materials_Buildings_Ignore_Enemies && pBuilding->m_iTeamNum() != pLocal->m_iTeamNum())
                    continue;
            }
            if (!F::VisualUtils->IsOnScreen(pLocal, pBuilding))
                continue;
            Color_t entColor;
            Color_t overlayColor;
            if (bIsLocal)
            {
                entColor = CFG::Color_Buildings_Local;
                overlayColor = CFG::Color_Buildings_Overlay_Local;
            }
            else if (pBuilding->m_iTeamNum() == pLocal->m_iTeamNum())
            {
                entColor = CFG::Color_Buildings_Teammates;
                overlayColor = CFG::Color_Buildings_Overlay_Teammates;
            }
            else
            {
                entColor = CFG::Color_Buildings_Enemies;
                overlayColor = CFG::Color_Buildings_Overlay_Enemies;
            }
            IMaterial* pMaterial = CFG::Materials_Buildings_Material == 0 ? nullptr : GetMaterial(CFG::Materials_Buildings_Material, false);
            IMaterial* pMaterialHidden = (CFG::Materials_Buildings_HiddenMaterial && CFG::Materials_Buildings_Material != 0) ? GetMaterial(CFG::Materials_Buildings_Material, true) : nullptr;
            int overlayType = CFG::Materials_Buildings_TwoModels;
            IMaterial* pOverlay = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, false);
            IMaterial* pOverlayHidden = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, true);
            auto DrawBuilding = [&]()
                {
                    DrawEntity(pBuilding);
                };
            if (pMaterialHidden)
            {
                I::ModelRender->ForcedMaterialOverride(pMaterialHidden);
                if (pMaterialHidden != m_pGlow && pMaterialHidden != m_pGlowIgnoreZ)
                    I::RenderView->SetColorModulation(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
                else if (pMaterialHidden == m_pGlowIgnoreZ)
                    m_pGlowIgnoreZEnvmapTint->SetVecValue(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
                DrawBuilding();
            }
            if (CFG::Materials_Buildings_No_Depth)
                pRenderContext->DepthRange(0.0f, 0.2f);
            I::ModelRender->ForcedMaterialOverride(pMaterial);
            if (pMaterial && pMaterial != m_pGlow)
                I::RenderView->SetColorModulation(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
            else if (pMaterial == m_pGlow)
                m_pGlowEnvmapTint->SetVecValue(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
            DrawBuilding();
            if (overlayType > 0)
            {
                I::ModelRender->ForcedMaterialOverride(pOverlayHidden);
                I::RenderView->SetColorModulation(ColorUtils::ToFloat(overlayColor.r), ColorUtils::ToFloat(overlayColor.g), ColorUtils::ToFloat(overlayColor.b));
                DrawBuilding();
                I::ModelRender->ForcedMaterialOverride(pOverlay);
                I::RenderView->SetColorModulation(ColorUtils::ToFloat(overlayColor.r), ColorUtils::ToFloat(overlayColor.g), ColorUtils::ToFloat(overlayColor.b));
                DrawBuilding();
            }
            if (CFG::Materials_Buildings_No_Depth)
                pRenderContext->DepthRange(0.0f, 1.0f);
        }
        I::ModelRender->ForcedMaterialOverride(nullptr);
        I::RenderView->SetBlend(1.0f);
    }
    if (CFG::Materials_World_Active)
    {
        I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
        if (CFG::Materials_World_Alpha < 1.0f)
            I::RenderView->SetBlend(CFG::Materials_World_Alpha);
        IMaterial* pMaterial = CFG::Materials_World_Material == 0 ? nullptr : GetMaterial(CFG::Materials_World_Material, false);
        IMaterial* pMaterialHidden = (CFG::Materials_World_HiddenMaterial && CFG::Materials_World_Material != 0) ? GetMaterial(CFG::Materials_World_Material, true) : nullptr;
        int overlayType = CFG::Materials_World_TwoModels;
        IMaterial* pOverlay = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, false);
        IMaterial* pOverlayHidden = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, true);
        auto ApplyMaterialAndDraw = [&](IMaterial* mat, const Color_t& color, C_BaseEntity* pEntity)
            {
                I::ModelRender->ForcedMaterialOverride(mat);
                if (mat && mat != m_pGlow && mat != m_pGlowIgnoreZ)
                    I::RenderView->SetColorModulation(ColorUtils::ToFloat(color.r), ColorUtils::ToFloat(color.g), ColorUtils::ToFloat(color.b));
                else if (mat == m_pGlow)
                    m_pGlowEnvmapTint->SetVecValue(ColorUtils::ToFloat(color.r), ColorUtils::ToFloat(color.g), ColorUtils::ToFloat(color.b));
                else if (mat == m_pGlowIgnoreZ)
                    m_pGlowIgnoreZEnvmapTint->SetVecValue(ColorUtils::ToFloat(color.r), ColorUtils::ToFloat(color.g), ColorUtils::ToFloat(color.b));
                DrawEntity(pEntity);
            };
        if (!CFG::Materials_World_Ignore_HealthPacks)
        {
            const auto color = CFG::Color_HealthPack;
            const auto overlayColor = CFG::Color_HealthPack_Overlay;
            for (const auto pEntity : H::Entities->GetGroup(EEntGroup::HEALTHPACKS))
            {
                if (!pEntity || !F::VisualUtils->IsOnScreen(pLocal, pEntity))
                    continue;
                if (pMaterialHidden)
                    ApplyMaterialAndDraw(pMaterialHidden, color, pEntity);
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 0.2f);
                ApplyMaterialAndDraw(pMaterial, color, pEntity);
                if (overlayType > 0)
                {
                    ApplyMaterialAndDraw(pOverlayHidden, overlayColor, pEntity);
                    ApplyMaterialAndDraw(pOverlay, overlayColor, pEntity);
                }
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 1.0f);
            }
        }
        if (!CFG::Materials_World_Ignore_AmmoPacks)
        {
            const auto color = CFG::Color_AmmoPack;
            const auto overlayColor = CFG::Color_AmmoPack_Overlay;
            for (const auto pEntity : H::Entities->GetGroup(EEntGroup::AMMOPACKS))
            {
                if (!pEntity || !F::VisualUtils->IsOnScreen(pLocal, pEntity))
                    continue;
                if (pMaterialHidden)
                    ApplyMaterialAndDraw(pMaterialHidden, color, pEntity);
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 0.2f);
                ApplyMaterialAndDraw(pMaterial, color, pEntity);
                if (overlayType > 0)
                {
                    ApplyMaterialAndDraw(pOverlayHidden, overlayColor, pEntity);
                    ApplyMaterialAndDraw(pOverlay, overlayColor, pEntity);
                }
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 1.0f);
            }
        }
        if (!CFG::Materials_World_Ignore_Halloween_Gift)
        {
            const auto color = CFG::Color_Halloween_Gift;
            const auto overlayColor = CFG::Color_Halloween_Gift_Overlay;
            for (const auto pEntity : H::Entities->GetGroup(EEntGroup::HALLOWEEN_GIFT))
            {
                if (!pEntity || !pEntity->ShouldDraw() || !F::VisualUtils->IsOnScreen(pLocal, pEntity))
                    continue;
                if (pMaterialHidden)
                    ApplyMaterialAndDraw(pMaterialHidden, color, pEntity);
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 0.2f);
                ApplyMaterialAndDraw(pMaterial, color, pEntity);
                if (overlayType > 0)
                {
                    ApplyMaterialAndDraw(pOverlayHidden, overlayColor, pEntity);
                    ApplyMaterialAndDraw(pOverlay, overlayColor, pEntity);
                }
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 1.0f);
            }
        }
        if (!CFG::Materials_World_Ignore_MVM_Money)
        {
            const auto color = CFG::Color_MVM_Money;
            const auto overlayColor = CFG::Color_MVM_Money_Overlay;
            for (const auto pEntity : H::Entities->GetGroup(EEntGroup::MVM_MONEY))
            {
                if (!pEntity || !pEntity->ShouldDraw() || !F::VisualUtils->IsOnScreen(pLocal, pEntity))
                    continue;
                if (pMaterialHidden)
                    ApplyMaterialAndDraw(pMaterialHidden, color, pEntity);
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 0.2f);
                ApplyMaterialAndDraw(pMaterial, color, pEntity);
                if (overlayType > 0)
                {
                    ApplyMaterialAndDraw(pOverlayHidden, overlayColor, pEntity);
                    ApplyMaterialAndDraw(pOverlay, overlayColor, pEntity);
                }
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 1.0f);
            }
        }
        const bool bIgnoringAllProjectiles = CFG::Materials_World_Ignore_LocalProjectiles
            && CFG::Materials_World_Ignore_EnemyProjectiles
            && CFG::Materials_World_Ignore_TeammateProjectiles;
        if (!bIgnoringAllProjectiles)
        {
            for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PROJECTILES_ALL))
            {
                if (!pEntity || !pEntity->ShouldDraw())
                    continue;
                const bool bIsLocal = F::VisualUtils->IsEntityOwnedBy(pEntity, pLocal);
                if (CFG::Materials_World_Ignore_LocalProjectiles && bIsLocal)
                    continue;
                if (!bIsLocal)
                {
                    if (CFG::Materials_World_Ignore_EnemyProjectiles && pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
                        continue;
                    if (CFG::Materials_World_Ignore_TeammateProjectiles && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
                        continue;
                }
                if (!F::VisualUtils->IsOnScreen(pLocal, pEntity))
                    continue;
                Color_t color;
                Color_t overlayColor;
                if (bIsLocal)
                {
                    color = CFG::Color_Projectiles_Local;
                    overlayColor = CFG::Color_Projectiles_Overlay_Local;
                }
                else if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
                {
                    color = CFG::Color_Projectiles_Teammates;
                    overlayColor = CFG::Color_Projectiles_Overlay_Teammates;
                }
                else
                {
                    color = CFG::Color_Projectiles_Enemies;
                    overlayColor = CFG::Color_Projectiles_Overlay_Enemies;
                }
                if (pMaterialHidden)
                    ApplyMaterialAndDraw(pMaterialHidden, color, pEntity);
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 0.2f);
                ApplyMaterialAndDraw(pMaterial, color, pEntity);
                if (overlayType > 0)
                {
                    ApplyMaterialAndDraw(pOverlayHidden, overlayColor, pEntity);
                    ApplyMaterialAndDraw(pOverlay, overlayColor, pEntity);
                }
                if (CFG::Materials_World_No_Depth)
                    pRenderContext->DepthRange(0.0f, 1.0f);
            }
        }
        I::ModelRender->ForcedMaterialOverride(nullptr);
        I::RenderView->SetBlend(1.0f);
    }
    if (!pLocal || pLocal->deadflag())
        return;
    const auto hViewModel = pLocal->m_hViewModel();
    if (!hViewModel)
        return;
    C_BaseEntity* pViewModel =
        static_cast<C_BaseEntity*>(
            I::ClientEntityList->GetClientEntityFromHandle(hViewModel));
    if (!pViewModel)
        return;
    // ================= DEFINIÇÃO CORRETA =================
    // ViewModel em si = HANDS
    C_BaseEntity* pHands = pViewModel;
    // Filhos do ViewModel = WEAPON / attachments
    C_BaseEntity* pWeapon = pViewModel->FirstMoveChild();
    // =====================================================
// ================= HANDS =================
    if (CFG::Materials_Hands_Active)
    {
        const auto entColor = CFG::Color_Hands;
        const auto overlayColor = CFG::Color_Hands_Overlay;
        I::RenderView->SetBlend(CFG::Materials_Hands_Alpha);
        IMaterial* pMaterial = CFG::Materials_Hands_Material == 0 ? nullptr : GetMaterial(CFG::Materials_Hands_Material, false);
        IMaterial* pMaterialHidden = CFG::Materials_Hands_HiddenMaterial && CFG::Materials_Hands_Material != 0 ? GetMaterial(CFG::Materials_Hands_Material, true) : nullptr;
        int overlayType = CFG::Materials_Hands_TwoModels;
        IMaterial* pOverlay = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, false);
        IMaterial* pOverlayHidden = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, true);
        auto DrawHands = [&]()
            {
                DrawEntity(pHands);
            };
        if (pMaterialHidden)
        {
            I::ModelRender->ForcedMaterialOverride(pMaterialHidden);
            if (pMaterialHidden != m_pGlow && pMaterialHidden != m_pGlowIgnoreZ)
                I::RenderView->SetColorModulation(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
            else if (pMaterialHidden == m_pGlowIgnoreZ)
                m_pGlowIgnoreZEnvmapTint->SetVecValue(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
            DrawHands();
        }
        if (CFG::Materials_Hands_No_Depth)
            pRenderContext->DepthRange(0.0f, 0.2f);
        I::ModelRender->ForcedMaterialOverride(pMaterial);
        if (pMaterial && pMaterial != m_pGlow)
            I::RenderView->SetColorModulation(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
        else if (pMaterial == m_pGlow)
            m_pGlowEnvmapTint->SetVecValue(ColorUtils::ToFloat(entColor.r), ColorUtils::ToFloat(entColor.g), ColorUtils::ToFloat(entColor.b));
        DrawHands();
        if (overlayType > 0)
        {
            I::ModelRender->ForcedMaterialOverride(pOverlayHidden);
            I::RenderView->SetColorModulation(ColorUtils::ToFloat(overlayColor.r), ColorUtils::ToFloat(overlayColor.g), ColorUtils::ToFloat(overlayColor.b));
            DrawHands();
            I::ModelRender->ForcedMaterialOverride(pOverlay);
            I::RenderView->SetColorModulation(ColorUtils::ToFloat(overlayColor.r), ColorUtils::ToFloat(overlayColor.g), ColorUtils::ToFloat(overlayColor.b));
            DrawHands();
        }
        if (CFG::Materials_Hands_No_Depth)
            pRenderContext->DepthRange(0.0f, 1.0f);
    }
    // ================= WEAPON =================
    if (CFG::Materials_Weapons_Active)
    {
        const auto color = CFG::Color_Weapons;
        const auto overlayColor = CFG::Color_Weapons_Overlay;
        I::RenderView->SetBlend(CFG::Materials_Weapons_Alpha);
        IMaterial* pMaterial = CFG::Materials_Weapons_Material == 0 ? nullptr : GetMaterial(CFG::Materials_Weapons_Material, false);
        IMaterial* pMaterialHidden = CFG::Materials_Weapons_HiddenMaterial && CFG::Materials_Weapons_Material != 0 ? GetMaterial(CFG::Materials_Weapons_Material, true) : nullptr;
        int overlayType = CFG::Materials_Weapons_TwoModels;
        IMaterial* pOverlay = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, false);
        IMaterial* pOverlayHidden = overlayType == 0 ? nullptr : GetMaterial(overlayType + 6, true);
        auto DrawWeapons = [&]()
            {
                for (C_BaseEntity* pAttach = pWeapon; pAttach; pAttach = pAttach->NextMovePeer())
                {
                    if (pAttach->ShouldDraw())
                        DrawEntity(pAttach);
                }
            };
        if (pMaterialHidden)
        {
            I::ModelRender->ForcedMaterialOverride(pMaterialHidden);
            if (pMaterialHidden != m_pGlow && pMaterialHidden != m_pGlowIgnoreZ)
                I::RenderView->SetColorModulation(ColorUtils::ToFloat(color.r), ColorUtils::ToFloat(color.g), ColorUtils::ToFloat(color.b));
            else if (pMaterialHidden == m_pGlowIgnoreZ)
                m_pGlowIgnoreZEnvmapTint->SetVecValue(ColorUtils::ToFloat(color.r), ColorUtils::ToFloat(color.g), ColorUtils::ToFloat(color.b));
            DrawWeapons();
        }
        if (CFG::Materials_Weapons_No_Depth)
            pRenderContext->DepthRange(0.0f, 0.2f);
        I::ModelRender->ForcedMaterialOverride(pMaterial);
        if (pMaterial && pMaterial != m_pGlow)
            I::RenderView->SetColorModulation(ColorUtils::ToFloat(color.r), ColorUtils::ToFloat(color.g), ColorUtils::ToFloat(color.b));
        else if (pMaterial == m_pGlow)
            m_pGlowEnvmapTint->SetVecValue(ColorUtils::ToFloat(color.r), ColorUtils::ToFloat(color.g), ColorUtils::ToFloat(color.b));
        DrawWeapons();
        if (overlayType > 0)
        {
            I::ModelRender->ForcedMaterialOverride(pOverlayHidden);
            I::RenderView->SetColorModulation(ColorUtils::ToFloat(overlayColor.r), ColorUtils::ToFloat(overlayColor.g), ColorUtils::ToFloat(overlayColor.b));
            DrawWeapons();
            I::ModelRender->ForcedMaterialOverride(pOverlay);
            I::RenderView->SetColorModulation(ColorUtils::ToFloat(overlayColor.r), ColorUtils::ToFloat(overlayColor.g), ColorUtils::ToFloat(overlayColor.b));
            DrawWeapons();
        }
        if (CFG::Materials_Weapons_No_Depth)
            pRenderContext->DepthRange(0.0f, 1.0f);
    }
    // ================= RESTAURA ESTADO =================
    I::ModelRender->ForcedMaterialOverride(nullptr);
    I::RenderView->SetBlend(1.f);
    pRenderContext->DepthRange(0.f, 1.f);
}

void CMaterials::CleanUp()
{
    m_bCleaningUp = true;
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
    if (m_pFlatNoInvis)
    {
        m_pFlatNoInvis->DecrementReferenceCount();
        m_pFlatNoInvis->DeleteIfUnreferenced();
        m_pFlatNoInvis = nullptr;
    }
    if (m_pShadedNoInvis)
    {
        m_pShadedNoInvis->DecrementReferenceCount();
        m_pShadedNoInvis->DeleteIfUnreferenced();
        m_pShadedNoInvis = nullptr;
    }
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