#pragma once
#include <future>
#include <tuple>

template<typename F, typename Tuple, size_t ...S >
decltype(auto) apply_tuple_impl(F&& fn, Tuple&& t, std::index_sequence<S...>)
{
  return std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
}

/*!
 * Apply arguments stored in a tuple ot a function. Can be replaced with std::apply from c++17
 * https://cpppatterns.com/patterns/apply-tuple-to-function.html
*/
template<typename F, typename Tuple>
decltype(auto) apply_from_tuple(F&& fn, Tuple&& t)
{
  std::size_t constexpr tSize
    = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
  return apply_tuple_impl(std::forward<F>(fn),
                          std::forward<Tuple>(t),
                          std::make_index_sequence<tSize>());
}

/*!
 *  TaskWrapper wraps a functor with parameters to be executed later.
 *  Supports arbitrary arguments and returns the result through promise-future.
 */
template<typename T, class ...Args>
class TaskWrapper {
public:
    TaskWrapper(std::function<T(Args...)> impl, Args... args);
    void operator()();
    std::future<T> getFuture();

private:
    template<typename K> struct type { };
    void runFunction(type<void>);
    template<typename K> void runFunction(type<K>);

private:
    std::function<T(Args...)> m_impl;
    std::tuple<Args...> m_args;
    std::shared_ptr<std::promise<T>> m_promisePtr;
};

template<class T, class ...Args>
TaskWrapper<T, Args...>::TaskWrapper(std::function<T(Args...)> impl, Args... args)
    : m_impl(impl)
    , m_args(std::forward<Args>(args)...)
    , m_promisePtr(std::make_shared<std::promise<T>>())
{
}

template<class T, class ...Args>
void TaskWrapper<T, Args...>::operator()(){
    runFunction(TaskWrapper::type<T>());
}

template<class T, class ...Args> template<typename K>
void TaskWrapper<T, Args...>::runFunction(type<K>) {
    m_promisePtr->set_value(apply_from_tuple(m_impl, m_args)); // std::apply (c++17)
}

template<class T, class ...Args>
void TaskWrapper<T, Args...>::runFunction(type<void>) {
    apply_from_tuple(m_impl, m_args);
    m_promisePtr->set_value(); // std::apply (c++17)
}


template<class T, class ...Args>
std::future<T> TaskWrapper<T, Args...>::getFuture(){
    return m_promisePtr->get_future();
}
