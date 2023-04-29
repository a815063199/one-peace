#pragma once

class BaseClass {
public:
    virtual ~BaseClass() {}

    virtual bool process()  = 0;
};

class PocessorRegistry {
public:
    static void regist_class(const std::string& processor_name, std::function<std::unique_ptr<BaseClass>()> func) {
        _registry_table.emplace(processor_name, func);
    }

    static std::unique_ptr<BaseClass> create_processor(const std::string& processor_name) {
        if (_registry_table.count(processor_name) > 0) {
            return _registry_table.at(processor_name)();
        }
        return nullptr;
    }
private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<BaseClass>()>> _registry_table;
};

class RegistSugar {
public:
    RegistSugar(const std::string& name, std::function<std::unique_ptr<BaseClass>()> func) {
        PocessorRegistry::regist_class(name, func);
    }
};

#define REGIST_PROCESSOR(processor_name, class_name) \
    static std::unique_ptr<BaseClass> get_class_##processor_name() {\
        return std::make_unique<class_name>();\
    }\
    static RegistSugar g_regist_sugar_##class_name(#processor_name, get_class_##processor_name);

