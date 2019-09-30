#pragma once

//Guaranteed to be lazy initialized and that it will be destroyed correctly
//Copy constructor and assignement operator should not be implemented
//DO NOT define public constructors but implement the private one (and destructor)
#define BZ_GENERATE_SINGLETON(Type) public:\
static Type& getInstance() {\
	static Type instance;\
	return instance;\
}\
private:\
Type();\
~Type();\
Type(const Type &copy) = delete;\
Type& operator=(const Type &copy) = delete;


/*template<typename T>
class LazySingleton {
    static T& get() {
        static T instance;
        return instance;
    }

private:
    T();
    ~T();
    T(const T& copy) = delete;
    T& operator=(const T& copy) = delete;
};


template<typename T>
class Singleton {
    static T& get() {
        return getImpl();
    }

    template<typename Args>
    static void init(Args& args...) {
        getImpl(args);
    }

private:
    template<typename Args>
    static T& getImpl(Args& args...) {
        static T instance(args);
        return instance;
    }

    template<typename Args>
    T(Args& args...);
    ~T();
    T(const T& copy) = delete;
    T& operator=(const T& copy) = delete;
};*/