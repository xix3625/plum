#pragma once

#include "Define.h"
#include <list>

struct stRect
{
	stRect()
		: left(0), right(0), bottom(0), top(0)
	{}

	int32		left;
	int32		right;
	int32		bottom;
	int32		top;
};

class SectorManager
{
public:
	typedef std::list<uint32>		LIST_SECTOR;

public:
	SectorManager(const tid mapTID);
	~SectorManager();
	bool							Initialize(const int32 viewRadius);
	uint32							GetSectorID(const int x, const int y);
	bool							GetSectorList(LIST_SECTOR& rList, const uint32 sectorID);
	bool							GetHittableSectorList(LIST_SECTOR& rList, const uint32 standardSectorID, const uint32 range);
	void							GetSectorRect(stRect& rRect, const uint32 sectorID, const uint32 range);

private:
	tid								_mapTID;
	bool							_bLump;
	int32							_viewRadius;
	int32							_columnCount;
	int32							_rowCount;
	int32							_totalCount;
	float							_offSetX;
	float							_offSetY;
	float							_paddingY;
};
