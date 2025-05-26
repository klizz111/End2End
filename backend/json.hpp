#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace nlohmann {
    class json {
    private:
        enum Type { OBJECT, ARRAY, STRING, NUMBER, BOOLEAN, NULL_TYPE };
        Type type;
        std::map<std::string, json> obj;
        std::vector<json> arr;
        std::string str;
        double num;
        bool boolean;
        
    public:
        json() : type(NULL_TYPE) {}
        json(const std::string& s) : type(STRING), str(s) {}
        json(const char* s) : type(STRING), str(s) {}
        json(double n) : type(NUMBER), num(n) {}
        json(int n) : type(NUMBER), num(n) {}
        json(bool b) : type(BOOLEAN), boolean(b) {}
        
        static json object() {
            json j;
            j.type = OBJECT;
            return j;
        }
        
        static json array() {
            json j;
            j.type = ARRAY;
            return j;
        }
        
        json& operator[](const std::string& key) {
            if (type == NULL_TYPE) type = OBJECT;
            return obj[key];
        }
        
        const json& operator[](const std::string& key) const {
            return obj.at(key);
        }
        
        void push_back(const json& value) {
            if (type == NULL_TYPE) type = ARRAY;
            arr.push_back(value);
        }
        
        bool contains(const std::string& key) const {
            return type == OBJECT && obj.find(key) != obj.end();
        }
        
        template<typename T>
        T get() const {
            if constexpr (std::is_same_v<T, std::string>) {
                return str;
            } else if constexpr (std::is_same_v<T, int>) {
                return static_cast<int>(num);
            } else if constexpr (std::is_same_v<T, double>) {
                return num;
            } else if constexpr (std::is_same_v<T, bool>) {
                return boolean;
            }
        }
        
        std::string dump() const {
            std::stringstream ss;
            switch (type) {
                case OBJECT:
                    ss << "{";
                    for (auto it = obj.begin(); it != obj.end(); ++it) {
                        if (it != obj.begin()) ss << ",";
                        ss << "\"" << it->first << "\":" << it->second.dump();
                    }
                    ss << "}";
                    break;
                case ARRAY:
                    ss << "[";
                    for (size_t i = 0; i < arr.size(); ++i) {
                        if (i > 0) ss << ",";
                        ss << arr[i].dump();
                    }
                    ss << "]";
                    break;
                case STRING:
                    ss << "\"" << str << "\"";
                    break;
                case NUMBER:
                    ss << num;
                    break;
                case BOOLEAN:
                    ss << (boolean ? "true" : "false");
                    break;
                case NULL_TYPE:
                    ss << "null";
                    break;
            }
            return ss.str();
        }
        
        static json parse(const std::string& text) {
            // 简化的JSON解析器
            json result;
            size_t pos = 0;
            return parseValue(text, pos);
        }
        
    private:
        static json parseValue(const std::string& text, size_t& pos) {
            skipWhitespace(text, pos);
            
            if (pos >= text.length()) return json();
            
            char c = text[pos];
            if (c == '{') {
                return parseObject(text, pos);
            } else if (c == '[') {
                return parseArray(text, pos);
            } else if (c == '"') {
                return parseString(text, pos);
            } else if (c == 't' || c == 'f') {
                return parseBoolean(text, pos);
            } else if (c == 'n') {
                return parseNull(text, pos);
            } else if (isdigit(c) || c == '-') {
                return parseNumber(text, pos);
            }
            return json();
        }
        
        static json parseObject(const std::string& text, size_t& pos) {
            json obj = json::object();
            pos++; // skip '{'
            skipWhitespace(text, pos);
            
            if (pos < text.length() && text[pos] == '}') {
                pos++;
                return obj;
            }
            
            while (pos < text.length()) {
                skipWhitespace(text, pos);
                json key = parseString(text, pos);
                skipWhitespace(text, pos);
                if (pos < text.length() && text[pos] == ':') pos++;
                skipWhitespace(text, pos);
                json value = parseValue(text, pos);
                obj[key.get<std::string>()] = value;
                
                skipWhitespace(text, pos);
                if (pos < text.length() && text[pos] == ',') {
                    pos++;
                } else if (pos < text.length() && text[pos] == '}') {
                    pos++;
                    break;
                }
            }
            return obj;
        }
        
        static json parseArray(const std::string& text, size_t& pos) {
            json arr = json::array();
            pos++; // skip '['
            skipWhitespace(text, pos);
            
            if (pos < text.length() && text[pos] == ']') {
                pos++;
                return arr;
            }
            
            while (pos < text.length()) {
                skipWhitespace(text, pos);
                json value = parseValue(text, pos);
                arr.push_back(value);
                
                skipWhitespace(text, pos);
                if (pos < text.length() && text[pos] == ',') {
                    pos++;
                } else if (pos < text.length() && text[pos] == ']') {
                    pos++;
                    break;
                }
            }
            return arr;
        }
        
        static json parseString(const std::string& text, size_t& pos) {
            pos++; // skip '"'
            std::string value;
            while (pos < text.length() && text[pos] != '"') {
                value += text[pos++];
            }
            if (pos < text.length()) pos++; // skip closing '"'
            return json(value);
        }
        
        static json parseNumber(const std::string& text, size_t& pos) {
            std::string numStr;
            while (pos < text.length() && (isdigit(text[pos]) || text[pos] == '.' || text[pos] == '-' || text[pos] == 'e' || text[pos] == 'E' || text[pos] == '+')) {
                numStr += text[pos++];
            }
            return json(std::stod(numStr));
        }
        
        static json parseBoolean(const std::string& text, size_t& pos) {
            if (text.substr(pos, 4) == "true") {
                pos += 4;
                return json(true);
            } else if (text.substr(pos, 5) == "false") {
                pos += 5;
                return json(false);
            }
            return json();
        }
        
        static json parseNull(const std::string& text, size_t& pos) {
            if (text.substr(pos, 4) == "null") {
                pos += 4;
            }
            return json();
        }
        
        static void skipWhitespace(const std::string& text, size_t& pos) {
            while (pos < text.length() && (text[pos] == ' ' || text[pos] == '\t' || text[pos] == '\n' || text[pos] == '\r')) {
                pos++;
            }
        }
    };
}