#include <exception>


Serializer::Serializer(std::ostream& out) : out_(out)
{
}


template <class T>
Error Serializer::save(T& object)
{
    return object.serialize(*this);
}


template <typename T, class... ArgsT>
Error Serializer::operator()(const T &arg, const ArgsT & ...args)
{
    Error res = (*this)(arg);
    if (res == Error::CorruptedArchive) {
        return res;
    }
    return (*this)(args...);
}


template <typename T>
Error Serializer::operator()(const T &arg)
{
    throw std::bad_typeid();
}


Error Serializer::operator()(bool arg)
{
    if (arg) {
        out_ << "true" << Serializer::Separator;
    } else {
        out_ << "false" << Serializer::Separator;
    }

    return Error::NoError;
}


Error Serializer::operator()(uint64_t arg)
{
    out_ << arg << Serializer::Separator;
    return Error::NoError;
}


Deserializer::Deserializer(std::istream& in) : in_(in)
{
}


template <class T>
Error Deserializer::load(T& object)
{
    return object.deserialize(*this);
}


template <typename T, class... ArgsT>
Error Deserializer::operator()(T &arg, ArgsT &... args)
{
    Error res = (*this)(arg);
    if (res == Error::CorruptedArchive) {
        return res;
    }
    return (*this)(args...);
}


template <typename T>
Error Deserializer::operator()(T &arg)
{
    throw std::bad_typeid();
}


Error Deserializer::operator()(bool &arg)
{
    std::string text;
    in_ >> text;
    if (text == "true") {
        arg = true;
    } else if (text == "false") {
        arg = false;
    } else {
        return Error::CorruptedArchive;
    }
    return Error::NoError;
}


Error Deserializer::operator()(uint64_t &arg)
{
    std::string tmp;
    in_ >> tmp;
    bool found_minus = false;
    if (tmp.find('-') != std::string::npos) {
        found_minus = true;
    }
    try {
        arg = std::stoull(tmp);
        if (found_minus && arg != 0) {
            return Error::CorruptedArchive;
        }
    } catch (const std::invalid_argument &) {
        return Error::CorruptedArchive;
    } catch (const std::out_of_range &) {
        return Error::CorruptedArchive;
    }
    return Error::NoError;
}
