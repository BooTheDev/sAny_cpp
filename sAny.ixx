module;
#include <typeinfo>
#include <type_traits>
export module m_sAny;

//#############################
//##   FUNCTIONS-PROTOTYPE   ##
//#############################

export namespace ns_sAny {
class sAny;

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$   Helper   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

// Copy memory
void memcpy(void* _destination, void const* _source, std::size_t _count);


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$   sAny_cast   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

// sAny_cast<type_t>(std::move(sAny));
template <typename T, typename T_raw = std::enable_if_t<std::is_rvalue_reference_v<T>, std::remove_reference_t<T>>>
T_raw&& sAny_cast(sAny&& obj);

// sAny_cast<type_t...>(Any); -> type_t...
template <typename T, typename T_raw = std::enable_if_t<!std::is_rvalue_reference_v<T>, std::remove_reference_t<T>>>
T sAny_cast(sAny& obj);

// sAny_cast<type_t...>(std::as_const(Any)); -> type_t const...
template <typename T, typename T_raw = std::enable_if_t<!std::is_rvalue_reference_v<T> && (!std::is_reference_v<T> || std::is_const_v<std::remove_reference_t<T>>), std::remove_reference_t<T>>>
T sAny_cast(sAny const& obj);

// sAny_cast<type_t>(&sAny);
template <typename T_raw>
T_raw* sAny_cast(sAny* const obj) noexcept;

// sAny_cast<type_t>(&sAny);
template <typename T_raw>
T_raw const* sAny_cast(sAny const* const obj) noexcept;

}

//#############################
//##    CLASS DECLARATION    ##
//#############################

namespace ns_sAny {

class sAny {
public:
    // Constructor / destructor
    sAny() = default;

    template<typename T>
    sAny(T&& obj) noexcept;

    ~sAny();

    // Get data
    bool has_value() const noexcept;
    std::type_info const& type() const noexcept;

    sAny& operator=(sAny&& obj) noexcept;

    sAny& operator=(sAny const& obj) noexcept;

    template <typename T, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<T>, sAny>>* = nullptr>
    sAny& operator=(T&& obj) noexcept;

    template <typename T>
    void bad_cast_checker() const noexcept(false);

    // sAny_cast
    template <typename T, typename T_raw>
    friend T_raw&& sAny_cast(sAny&& obj);

    template <typename T, typename T_raw>
    friend T sAny_cast(sAny& obj);

    template <typename T, typename T_raw>
    friend T sAny_cast(sAny const& obj);

    template <typename T_raw>
    friend T_raw* sAny_cast(sAny* const obj) noexcept;

    template <typename T_raw>
    friend T_raw const* sAny_cast(sAny const* const obj) noexcept;

private:
    void* m_ptr = nullptr;
    std::type_info const* m_type;
    std::size_t m_size = 0ui8;
};

}


//#############################
//##   FUNCTION DEFINITION   ##
//#############################

namespace ns_sAny {

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$   Helper   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void memcpy(void* _destination, void const* _source, std::size_t _count) {
    auto _dest = reinterpret_cast<uint8_t*>(_destination);
    auto _src = reinterpret_cast<uint8_t const*>(_source);
    for (++_count; --_count; ++_dest, ++_src) {
        *_dest = *_src;
    }
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$   Member-functions   $$$$$$$$$$$$$$$$$$$$$$$$$$$

template<typename T>
sAny::sAny(T&& obj) noexcept
{ this->operator=(static_cast<T&&>(obj)); }

sAny::~sAny()
{ if (m_ptr) delete m_ptr; }

bool sAny::has_value() const noexcept
{ return m_ptr != nullptr; }

std::type_info const& sAny::type() const noexcept
{ return *m_type; }

sAny& sAny::operator=(sAny&& obj) noexcept {
    m_ptr = obj.m_ptr;
    m_size = obj.m_size;
    m_type = obj.m_type;
    obj.m_ptr = nullptr;
    obj.m_size = 0U;
    obj.m_type = &typeid(void);
    return *this;
}

sAny& sAny::operator=(sAny const& obj) noexcept {
    m_ptr = new uint8_t[obj.m_size];
    memcpy(m_ptr, obj.m_ptr, obj.m_size);
    m_size = obj.m_size;
    m_type = obj.m_type;
    return *this;
}

template <typename T, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<T>, sAny>>*>
sAny& sAny::operator=(T&& obj) noexcept {
    delete m_ptr;
    m_ptr = new std::remove_cvref_t<T>{ static_cast<T&&>(obj) };
    m_size = sizeof(T);
    m_type = &typeid(T);
    return *this;
}

template <typename T>
void sAny::bad_cast_checker() const noexcept(false) {
    if (!m_ptr) {
        throw std::bad_cast();
    }
    if (*m_type != typeid(T)) {
        throw std::bad_cast();
    }
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$   sAny_cast   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

template <typename T, typename T_raw>
T_raw&& sAny_cast(sAny&& obj) {
    obj.bad_cast_checker<T_raw>();
    return static_cast<T_raw&&>(*reinterpret_cast<T_raw*>(obj.m_ptr));
}

template <typename T, typename T_raw>
T sAny_cast(sAny& obj) {
    obj.bad_cast_checker<T_raw>();
    return static_cast<T>(*reinterpret_cast<T_raw*>(obj.m_ptr));
}

template <typename T, typename T_raw>
T sAny_cast(sAny const& obj) {
    obj.bad_cast_checker<T_raw>();
    return static_cast<T>(*reinterpret_cast<T_raw*>(obj.m_ptr));
}

template <typename T_raw>
T_raw* sAny_cast(sAny* const obj) noexcept {
    return reinterpret_cast<T_raw*>(obj->m_ptr);
}

template <typename T_raw>
T_raw const* sAny_cast(sAny const* const obj) noexcept {
    return reinterpret_cast<T_raw const*>(obj->m_ptr);
}

}