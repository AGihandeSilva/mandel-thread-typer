#pragma once
#include<queue>
#include<string>
#include <memory>
#include <optional>
#include <mutex>

template <typename T>
class SynchronizedData : public std::enable_shared_from_this<SynchronizedData<T>> {

public:
	bool PushMessage(T jsonString)
	{
		bool validatedJsonString = true; //TODO

		if (!validatedJsonString)
		{
			return false;
		}
		std::unique_lock dataLock(m_mutex);
		m_messages.push(jsonString);

		//TODO notification scheme?

		return true;
	}

	std::optional<T> PopMessage()
	{
		if (empty())
		{
			return {};
		}
		else
		{
			std::unique_lock dataLock(m_mutex);
			T message = m_messages.back();
			m_messages.pop();
			const bool validatedString = true; //TODO

			return validatedString ? std::optional<T>(message) : std::optional<T>();
		}
	}
	bool empty()
	{
		std::unique_lock dataLock(m_mutex);
		return m_messages.empty();
	}

private:

		std::queue<T> m_messages;
		std::mutex m_mutex;
};
