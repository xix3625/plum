#include "Sector.h"
#include "PathData.h"

SectorManager::SectorManager(const tid mapTID)
	: _mapTID(mapTID)
	, _bLump(false)
	, _viewRadius(0)
	, _columnCount(0)
	, _rowCount(0)
	, _totalCount(0)
	, _offSetX(0.0f)
	, _offSetY(0.0f)
	, _paddingY(0.0f)
{
}

SectorManager::~SectorManager()
{
}

bool SectorManager::Initialize(const int32 viewRadius)
{
	if (0 >= viewRadius)
	{
		assert(false);
		return false;
	}

	PathData* pPathData = nullptr;
	if (!pPathData)
	{
		assert(false);
		return false;
	}

	int32 view			= viewRadius / MAX_SECTOR_SIZE;
	bool bLump			= false;
	int32 columnCount	= pPathData->GetMapSize().y / MAX_SECTOR_SIZE + 1;
	int32 rowCount		= pPathData->GetMapSize().x / MAX_SECTOR_SIZE + 1;
	int32 totalCount	= columnCount * rowCount;
	
	if ((viewRadius >= rowCount) || (viewRadius >= columnCount))
	{
		bLump			= true;
		columnCount		= 1;
		rowCount		= 1;
		totalCount		= 1;
	}

	_bLump				= bLump;
	_viewRadius			= view;
	_columnCount		= columnCount;
	_rowCount			= rowCount;
	_totalCount			= totalCount;
	_offSetX			= pPathData->GetMinPos().x;
	_offSetY			= pPathData->GetMinPos().y;
	_paddingY			= pPathData->GetPaddingY();

	return true;
}

uint32 SectorManager::GetSectorID(const int x, const int y)
{
	if (_bLump)
	{
		return 0;
	}

	const uint32 id = (y * _rowCount) + x;
	if (id >= _totalCount)
	{
		return 0;
	}

	return id;
}

bool SectorManager::GetSectorList(LIST_SECTOR& rList, const uint32 sectorID)
{
	if (sectorID >= _totalCount)
	{
		return false;
	}

	stRect rect;
	GetSectorRect(rect, sectorID, _viewRadius);

	for (int32 y = rect.bottom; y <= rect.top; ++y)
	{
		for (int32 x = rect.left; x <= rect.right; ++x)
		{
			const uint32 id = GetSectorID(x, y);

			rList.push_back(id);
		}
	}

	return true;
}

bool SectorManager::GetHittableSectorList(LIST_SECTOR& rList, const uint32 standardSectorID, const uint32 range)
{
	if (standardSectorID >= _totalCount)
	{
		assert(false);
		return false;
	}

	stRect rect;
	GetSectorRect(rect, standardSectorID, range);

	rList.push_back(standardSectorID);

	for (int32 y = rect.bottom; y <= rect.top; ++y)
	{
		for (int32 x = rect.left; x <= rect.right; ++x)
		{
			const int32 id = GetSectorID(x, y);
			if (standardSectorID == id)
				continue;

			rList.push_back(id);
		}
	}

	return true;
}

void SectorManager::GetSectorRect(stRect& rRect, const uint32 sectorID, const uint32 range)
{
	if (sectorID >= _totalCount)
	{
		assert(false);
		return;
	}

	if (0 == range)
	{
		assert(false);
		return;
	}

	const int32 x		= sectorID % _rowCount;
	const int32 y		= sectorID / _rowCount;

	rRect.left			= x - range;
	rRect.right			= x + range;
	rRect.bottom		= y - range;
	rRect.top			= y + range;

	if (rRect.left < 0)
	{
		rRect.left = 0;
	}

	if (rRect.right >= _rowCount)
	{
		rRect.right = _rowCount - 1;
	}

	if (rRect.bottom < 0)
	{
		rRect.bottom = 0;
	}

	if (rRect.top >= _columnCount)
	{
		rRect.top = _columnCount - 1;
	}
}
