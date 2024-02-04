#ifndef _ZFDRIVER_H_
#define _ZFDRIVER_H_

template <typename T>
class Singleton
{
private:
	Singleton() = default;
	~Singleton() = default;
	Singleton(const Singleton&) = delete;
	Singleton(const Singleton&&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton& operator=(const Singleton&&) = delete;
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}
};

class ZfDriver
{
	friend class Singleton<ZfDriver>;
private:
	ZfDriver();
	~ZfDriver();
	ZfDriver(const ZfDriver&) = delete;
	ZfDriver(const ZfDriver&&) = delete;
	ZfDriver& operator=(const ZfDriver&) = delete;
	ZfDriver& operator=(const ZfDriver&&) = delete;

public:
	bool Install();
	void Uninstall();
};

#endif // !_ZFDRIVER_H_
