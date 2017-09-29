#include "IOThread.h"
#include "EventObject.h"
#include "Socket.h"

__declspec(thread)	id32	IOThread::IOThreadId = ThreadBase::InvalidId;

IOThread::IOThread(void)
	: _pNetwork(nullptr), _ioThreadId(ThreadBase::InvalidId), _pFailedHandleEventFunc(nullptr)
{

}

IOThread::~IOThread(void)
{

}

bool IOThread::Initialize(id32 id, INetwork* pNetwork, FailedHandleEventFunc pFailedHandleEventFunc)
{
	_ioThreadId = id;

	_pNetwork = pNetwork;
	_pFailedHandleEventFunc = pFailedHandleEventFunc;
	return true;
}

bool IOThread::Run()
{
	BOOL result = false;
	ULONG numberOfBytes = 0;
	EventContext* pEventContext = nullptr;
	EventObject* pEventObject = nullptr;

	if (!_pNetwork) {
		return false;
	}

	IOThread::IOThreadId = _ioThreadId;
#ifdef _DEBUG
	LOG_DEBUG(L"start, id: %u", GetId());
	//LOG_INFO(L"start, iothreadid: %u", IOThread::GetIOThreadId());
#endif

	while (true)
	{
		result = GetQueuedCompletionStatus(_pNetwork->IocpHandle(), &numberOfBytes, (PULONG_PTR)&pEventObject, (LPOVERLAPPED*)&pEventContext, INFINITE);

		if (!IsRun()) {
#ifdef _DEBUG
			LOG_DEBUG(L"stop, id: %u", GetId());
#endif
			break;
		}

		if (result)
		{
			if (_pNetwork->EndEvent() == pEventObject)
			{
				LOG_INFO(L"catch terminate event, id: %u", GetId());
				break;
			}

			if (pEventObject)
			{
				EventObjectPtr eventObjectPtr(pEventObject);
				if (!pEventObject->HandleEvent(pEventContext, numberOfBytes))
				{
					LOG_DEBUG(L"event object handle failed, id: %u", GetId());
					FailedHandleEvent(pEventObject, pEventContext, numberOfBytes);
				}
			}
			else {
				LOG_INFO(L"yield, id: %u", GetId());
			}
		}
		else if (pEventObject && pEventContext)
		{
			EventObjectPtr eventObjectPtr(pEventObject);
			FailedHandleEvent(pEventObject, pEventContext, numberOfBytes);
		}
		else
		{
			int errorCode = WSAGetLastError();
			if (WAIT_TIMEOUT != errorCode)
			{
				LOG_INFO(L"iocp error, error: %u", errorCode);
			}
		}
	}

#ifdef _DEBUG
	LOG_DEBUG(L"stopped, id: %u", GetId());
	//LOG_INFO(L"stopped, iothreadid: %u", IOThread::GetIOThreadId());
#endif

	return true;
}

void IOThread::Stop()
{
	PostQueuedCompletionStatus(_pNetwork->IocpHandle(), 0, (ULONG_PTR)_pNetwork->EndEvent(), NULL);
}

void IOThread::FailedHandleEvent(EventObject* pEventObject, EventContext* pEventContext, ulong param)
{
	switch (pEventObject->EventId())
	{
	case EVOBJ_SOCKET_ID:
		((Socket*)pEventObject)->HandleFailedEvent(pEventContext, param);
		break;
	default:
		break;
	}

	if (_pFailedHandleEventFunc)
		_pFailedHandleEventFunc(_pNetwork, pEventObject);
}