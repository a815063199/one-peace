#include <stddef.h>

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <vector>

/*try {...} catch {...} wrapper*/
#define CD_TRY try {

#define CD_CATCH                                            \
    } catch (const std::exception &e) {                     \
        std::cerr << e.what() << "\n";                      \
        return false;                                       \
    } catch (...) {                                         \
        std::cerr << "Occur unknown exception." << "\n";    \
        return false;                                       \
    }

static std::unordered_map<std::string, int> MemberOffset;

void split_str(const std::string& source, const std::string& delimiter, std::vector<std::string>& result)
{
    char * strc = new char[strlen(source.c_str())+1];
    strcpy(strc, source.c_str());
    char* tmpStr = strtok(strc, delimiter.c_str());
    while (tmpStr != NULL)
    {
        result.push_back(std::string(tmpStr));
        tmpStr = strtok(NULL, delimiter.c_str());
    }

    delete[] strc;
}

/* 用户自定义类型: Address */
struct Address {
    std::string province;
    std::string city;
};

/* 用户自定义类型: Person */
struct Person {
    std::string name;
    std::string sex;
    int age = 0;
    int id = 0;
    std::vector<std::string> phones;
    Address addr;

    void debug_print() const {
        std::cout << name << "|" 
                << sex << "|" 
                << age << "|"
                << id << "|";
        for (size_t i = 0; i < phones.size(); ++i) {
            if (i < (phones.size() - 1)) {
                std::cout << phones[i] << ",";
            } else {
                std::cout << phones[i] << "|";
            }
        }
        std::cout << addr.province << "," << addr.city << "\n";
    }
};


#define REGISTER_MEMBER(class_name, member_name) MemberOffset[#member_name] = offsetof(class_name, member_name)
template<typename T>
void* GET_MEMBER(const T& obj_name, const std::string& member_name) {
    return reinterpret_cast<void*>(reinterpret_cast<long>(&obj_name) + MemberOffset[member_name]);
}

/* base parser */
class Parser {
public:
    virtual ~Parser() = default;

    virtual bool parse(
            const std::string& str_value, void* value) const = 0;
};

using ParserPtr = std::unique_ptr<Parser>;

class StrParser: public Parser {
public:
    virtual bool parse(const std::string& str_value, void* value) const {
        CD_TRY

        std::string* real_value = static_cast<std::string*>(value);

        *real_value = str_value;

        CD_CATCH

        return true;
    }
};

class StrArrayParser: public Parser {
public:
    virtual bool parse(const std::string& str_value, void* value) const {
        CD_TRY

        std::vector<std::string>* real_value = static_cast<std::vector<std::string>*>(value);

        if (str_value.size() >= 2 && str_value[1] == ':') {
            std::vector<std::string> temp_segments;
            split_str(str_value, ":", temp_segments);
            if (temp_segments.size() >= 2) {
                std::vector<std::string> value_segments;
                split_str(temp_segments[1], ",", value_segments);
                for (const auto& v : value_segments) {
                    real_value->push_back(v);
                }
            }

        } else {
            real_value->push_back(str_value);
        }

        CD_CATCH

        return true;
    }
};

class IntParser: public Parser {
public:
    virtual bool parse(const std::string& str_value, void* value) const {
        CD_TRY

        int* real_value = static_cast<int*>(value);

        *real_value= stoi(str_value);

        CD_CATCH

        return true;
    }
};

class AddressParser: public Parser {
public:
    virtual bool parse(const std::string& str_value, void* value) const {
        CD_TRY

        Address* real_value = static_cast<Address*>(value);
        
        std::vector<std::string> segments;
        split_str(str_value, ",", segments);
        if (segments.size() >= 2) {
            StrParser().parse(segments[0], &(real_value->province));
            StrParser().parse(segments[1], &(real_value->city));
        } else {
            std::cerr << "Address format error: " << str_value << "\n";
            return false;
        }

        CD_CATCH

        return true;
    }
};

/* parser 工厂 */
template <typename ProductType, 
        typename CreatorType, 
        typename CreatorTypeKey = std::string>
class Factory {
private:
    typedef std::unique_ptr<CreatorType> CreatorTypePtr;
    typedef std::unique_ptr<ProductType> ProductTypePtr;
    typedef std::unordered_map<CreatorTypeKey, CreatorTypePtr> CreatorTypePtrTable;

public:
    virtual ~Factory() {}

    virtual void init() = 0;

    virtual ProductTypePtr create(const CreatorTypeKey& creator_key) const {
        typename CreatorTypePtrTable::const_iterator iter = 
                _creator_table.find(creator_key);
        if (iter != _creator_table.end()) {
            return iter->second->create(); 
        } else {
            std::cerr << "Not exist creator_key : " << creator_key << "\n";
            return ProductTypePtr(nullptr);
        }
    }

protected:
    template <typename SpecificCreatorType>
    bool _registed(const CreatorTypeKey &creator_key) {
        typename CreatorTypePtrTable::const_iterator iter = 
                _creator_table.find(creator_key);
        if (iter != _creator_table.end()) {
            std::cerr << "Contain creator_key : " << creator_key << "\n";
            return false;
        } else {
            CreatorTypePtr new_creator(new SpecificCreatorType());
            _creator_table.insert(std::make_pair(
                    creator_key, std::move(new_creator)));
        }

        return true;
    }

    CreatorTypePtrTable _creator_table;
};

class CreatorBase {
public: 
    virtual ~CreatorBase() = default;

    virtual ParserPtr create() const = 0;
};

template <typename ParserType>
class ParserCreator: public CreatorBase {
public:
    virtual ParserPtr create() const {
        return ParserPtr(new ParserType);
    }
};

class ParserFactory : public Factory<Parser, CreatorBase> {
public: 
    static ParserFactory& instance() {
        static ParserFactory factory;
        return factory;
    }

    virtual void init() {
        _registed<ParserCreator<IntParser>>("Int");
        _registed<ParserCreator<StrParser>>("Str");
        _registed<ParserCreator<StrArrayParser>>("StrArray");
        _registed<ParserCreator<AddressParser>>("Address");
    }

private:
	ParserFactory() = default;
	ParserFactory(ParserFactory& other) = delete;
	ParserFactory& operator=(ParserFactory& other) = delete;
};

/* 通用词表解析器 */
class CommonTextReader {
public:
    bool init(const std::string& type_desc_file_name) {
        CD_TRY

        std::ifstream ifs;
        ifs.open(type_desc_file_name);
        if (ifs.is_open()) {
            std::string line_data;
            while (std::getline(ifs, line_data)) {
                std::vector<std::string> segments;
                split_str(line_data, ":", segments);
                if (segments.size() >= 3) {
                    int column_index = stoi(segments[0]);
                    _column_name_table[column_index] = segments[1];
                    _column_type_table[column_index] = segments[2];
                } else {
                    std::cerr << "type description format error: " << line_data << "\n";
                    return false;
                }
            }
        } else {
            std::cerr << "Failed to open file: " << type_desc_file_name << "\n";
            return false;
        }

        CD_CATCH

        return true;
    }

    template<typename DataType>
    bool read_data(const std::string& file_name, std::vector<DataType>& data_list) const {
        CD_TRY
        
        std::ifstream ifs;
        ifs.open(file_name);
        if (ifs.is_open()) {
            std::string line_data;
            // 逐行读取
            while (std::getline(ifs, line_data)) {
                std::vector<std::string> line_segments;
                split_str(line_data, "\t", line_segments);
                // 每行逐个字段解析
                DataType temp_data;
                for (size_t i = 0; i < line_segments.size(); ++i) {
                    if (_column_type_table.count(i) > 0 && _column_name_table.count(i) > 0) {
                        ParserPtr parser = ParserFactory::instance().create(_column_type_table.at(i));
                        parser->parse(line_segments[i], GET_MEMBER(temp_data, _column_name_table.at(i)));
                    }
                }
                data_list.push_back(temp_data);
            }
        } else {
            std::cerr << "Failed to open file: " << file_name << "\n";
            return false;
        }

        CD_CATCH

        return true;
    }

private:
    std::unordered_map<int, std::string> _column_name_table;

    std::unordered_map<int, std::string> _column_type_table;

};



int main() {
    /* 注册Person字段 */
    REGISTER_MEMBER(Person, name);
    REGISTER_MEMBER(Person, sex);
    REGISTER_MEMBER(Person, age);
    REGISTER_MEMBER(Person, id);
    REGISTER_MEMBER(Person, phones);
    REGISTER_MEMBER(Person, addr);

    /* Parser工厂初始化 */
    ParserFactory::instance().init();

    CommonTextReader reader;
    reader.init("type.txt");

    std::vector<Person> person_list;
    reader.read_data("data.txt", person_list);

    for (const auto& p : person_list) {
        p.debug_print();
    }

    return 0;
}
