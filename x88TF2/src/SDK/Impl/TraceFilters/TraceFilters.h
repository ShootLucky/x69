// TraceFilters.h
#pragma once
#include "../../TF2/IEngineTrace.h"

class CTraceFilterHitscan : public CTraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pServerEntity, int contentsMask) override;

	TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}

	C_BaseEntity* m_pIgnore = nullptr;
};

class CTraceFilterWorldCustom : public CTraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pServerEntity, int contentsMask) override;

	TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}

	C_BaseEntity* m_pTarget = nullptr;
};

class CTraceFilterArc : public CTraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pServerEntity, int contentsMask) override;

	TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}
};

class CTraceFilterSimple : public CTraceFilter
{
public:
	CTraceFilterSimple(const IHandleEntity* passentity, int collisionGroup)
		: m_pPassEnt(passentity), m_iCollisionGroup(collisionGroup)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity* pHandleEntity, int contentsMask) override
	{
		return pHandleEntity != m_pPassEnt;
	}

	virtual TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}

	const IHandleEntity* m_pPassEnt;
	int m_iCollisionGroup;
};