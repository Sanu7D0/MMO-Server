#include "pch.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	int32 lockId = 0;
	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end()) {
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	} else {
		lockId = findIt->second;
	}

	if (!_lockStack.empty()) {
		const int32 prevId = _lockStack.top();
		if (lockId != prevId) {
			std::set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end()) {
				history.insert(lockId);
				CheckCycle();
			}
		}
	}

	_lockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard gurad(_lock);

	if (_lockStack.empty())
		CRASH("MULTIPLE_UNLOCK");
	
	int32 lockId = _nameToId[name];
	if (_lockStack.top() != lockId)
		CRASH("INVALID_UNLOCK");
	

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoverOrder = std::vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = std::vector<bool>(lockCount, false);
	_parent = std::vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; ++lockId)
		Dfs(lockId);
	
	_discoverOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 u)
{
	if (_discoverOrder[u] != -1)
		return;

	_discoverOrder[u] = _discoveredCount++;

	auto findIt = _lockHistory.find(u);
	if (findIt == _lockHistory.end()) {
		_finished[u] = true;
		return;
	}

	std::set<int32>& nextSet = findIt->second;
	for (int32 v : nextSet) {
		if (_discoverOrder[v] == -1) {
			_parent[v] = u;
			Dfs(v);
			continue;
		}

		if (_discoverOrder[u] < _discoverOrder[v])
			continue;

		// cycle
		if (_finished[v] == false) {
			printf("%s -> %s\n", _idToName[u], _idToName[v]);
			int32 current = u;
			while (true) {
				printf("%s -> %s\n", _idToName[u], _idToName[v]);
				current = _parent[current];
				if (current == v)
					break;
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[u] = true;
}
