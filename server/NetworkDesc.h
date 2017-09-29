#pragma once

struct NetworkDesc
{
	NetworkDesc()
	{
		nSockPoolSize = 10;
		bNagle = true;
		bZeroSocketBuffer = false;
	}

	int	nSockPoolSize;
	bool bNagle;
	bool bZeroSocketBuffer;
};