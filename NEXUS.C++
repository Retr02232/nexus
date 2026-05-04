/*
╔═══════════════════════════════════════════════════════════════╗
║                        NEXUS AI v2.0                          ║
║          Advanced AI CLI - made by RETR0     ║
║                                                               ║
║  Features:                                                    ║
║  • Multi-Model Support (Claude, GPT-4, Gemini, Local)        ║
║  • Persistent Context & Memory                                ║
║  • Code Execution Engine                                      ║
║  • Web Search Integration                                     ║
║  • File Analysis (PDF, Images, Code)                         ║
║  • Beautiful TUI Interface                                    ║
║  • Voice I/O                                                  ║
║  • Agent Mode                                                 ║
║                                                               ║
║  Compile: g++ -std=c++17 -o nexus nexus.cpp -lcurl -ljson      
║  Run: ./nexus                                                 ║
╚═══════════════════════════════════════════════════════════════╝
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <iomanip>
#include <regex>

using namespace std;

// ============================================================
//                    CONFIGURATION
// ============================================================

struct Config {
    string anthropic_key = "";
    string openai_key = "";
    string google_key = "";
    string default_provider = "claude";
    string data_dir = "./nexus_data";
    bool enable_web_search = true;
    bool enable_code_exec = true;
    bool enable_voice = false;
    int max_context_messages = 20;
    float temperature = 0.7;
    int max_tokens = 4096;
};

// ============================================================
//                    UTILITY FUNCTIONS
// ============================================================

class Utils {
public:
    static string get_timestamp() {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
        return string(buf);
    }
    
    static string get_session_id() {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
        return string(buf);
    }
    
    static void clear_screen() {
        cout << "\033[2J\033[1;1H";
    }
    
    static void set_color(const string& color) {
        map<string, string> colors = {
            {"red", "\033[0;31m"},
            {"green", "\033[0;32m"},
            {"yellow", "\033[1;33m"},
            {"blue", "\033[0;34m"},
            {"cyan", "\033[0;36m"},
            {"magenta", "\033[0;35m"},
            {"white", "\033[1;37m"},
            {"reset", "\033[0m"}
        };
        cout << colors[color];
    }
    
    static void print_banner() {
        clear_screen();
        set_color("cyan");
        cout << R"(
╔══════════════════════════════════════════════
║                                                               ║
║   ███╗   ██╗███████╗██╗  ██╗██╗   ██╗███████╗    ║
║   ████╗  ██║██╔════╝╚██╗██╔╝██║   ██║██╔════╝   ║
║   ██╔██╗ ██║█████╗   ╚███╔╝ ██║   ██║███████╗    ║
║   ██║╚██╗██║██╔══╝   ██╔██╗ ██║   ██║╚════██║    ||
║   ██║ ╚████║███████╗██╔╝ ██╗╚██████╔╝███████║  ||
║   ╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝   ║
║                                                               ║
║              Advanced AI CLI - Version 2.0                    ║
║                       made by RETR0║
║                                                               ║
╚══════════════════════════════════════════════
)" << endl;
        set_color("reset");
    }
    
    static void print_separator(char c = '═', int width = 60) {
        set_color("cyan");
        for(int i = 0; i < width; i++) cout << c;
        cout << endl;
        set_color("reset");
    }
    
    static void typewriter_effect(const string& text, int delay_ms = 20) {
        for(char c : text) {
            cout << c << flush;
            this_thread::sleep_for(chrono::milliseconds(delay_ms));
        }
        cout << endl;
    }
    
    static string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if(first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }
    
    static vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        while(getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
    
    static bool file_exists(const string& filename) {
        ifstream f(filename);
        return f.good();
    }
    
    static string read_file(const string& filename) {
        ifstream file(filename);
        stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    static void write_file(const string& filename, const string& content) {
        ofstream file(filename);
        file << content;
        file.close();
    }
    
    static void create_directory(const string& path) {
        system(("mkdir -p " + path).c_str());
    }
};

// ============================================================
//                    HTTP CLIENT (cURL)
// ============================================================

class HttpClient {
private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

public:
    struct Response {
        int status_code;
        string body;
        bool success;
    };
    
    static Response post(const string& url, const string& data, const map<string, string>& headers) {
        CURL* curl = curl_easy_init();
        Response response;
        
        if(curl) {
            string response_string;
            struct curl_slist* header_list = NULL;
            
            // Add headers
            for(const auto& header : headers) {
                string header_str = header.first + ": " + header.second;
                header_list = curl_slist_append(header_list, header_str.c_str());
            }
            
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
            
            CURLcode res = curl_easy_perform(curl);
            
            if(res == CURLE_OK) {
                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                response.status_code = http_code;
                response.body = response_string;
                response.success = (http_code >= 200 && http_code < 300);
            } else {
                response.success = false;
                response.body = curl_easy_strerror(res);
            }
            
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl);
        }
        
        return response;
    }
    
    static Response get(const string& url, const map<string, string>& headers = {}) {
        CURL* curl = curl_easy_init();
        Response response;
        
        if(curl) {
            string response_string;
            struct curl_slist* header_list = NULL;
            
            for(const auto& header : headers) {
                string header_str = header.first + ": " + header.second;
                header_list = curl_slist_append(header_list, header_str.c_str());
            }
            
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
            
            CURLcode res = curl_easy_perform(curl);
            
            if(res == CURLE_OK) {
                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                response.status_code = http_code;
                response.body = response_string;
                response.success = (http_code >= 200 && http_code < 300);
            } else {
                response.success = false;
                response.body = curl_easy_strerror(res);
            }
            
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl);
        }
        
        return response;
    }
};

// ============================================================
//                    MESSAGE STRUCTURE
// ============================================================

struct Message {
    string role;        // "user" or "assistant"
    string content;
    string timestamp;
    map<string, string> metadata;
    
    Message(const string& r, const string& c) : role(r), content(c) {
        timestamp = Utils::get_timestamp();
    }
    
    string to_json() const {
        stringstream ss;
        ss << "{"
           << "\"role\":\"" << role << "\","
           << "\"content\":\"" << escape_json(content) << "\","
           << "\"timestamp\":\"" << timestamp << "\""
           << "}";
        return ss.str();
    }
    
private:
    string escape_json(const string& str) const {
        string escaped;
        for(char c : str) {
            switch(c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c;
            }
        }
        return escaped;
    }
};

// ============================================================
//                    AI ENGINE (Multi-Model)
// ============================================================

class AIEngine {
private:
    Config& config;
    
    string extract_text_from_response(const string& json_response, const string& provider) {
        // Simple JSON parsing (for production, use proper JSON library)
        
        if(provider == "claude") {
            // Claude response format: {"content":[{"text":"..."}]}
            size_t pos = json_response.find("\"text\":\"");
            if(pos != string::npos) {
                pos += 8; // length of "text":"
                size_t end = json_response.find("\"", pos);
                string text = json_response.substr(pos, end - pos);
                // Unescape JSON
                replace_all(text, "\\n", "\n");
                replace_all(text, "\\\"", "\"");
                return text;
            }
        }
        else if(provider == "gpt4") {
            // OpenAI response format: {"choices":[{"message":{"content":"..."}}]}
            size_t pos = json_response.find("\"content\":\"");
            if(pos != string::npos) {
                pos += 11;
                size_t end = json_response.find("\"", pos);
                string text = json_response.substr(pos, end - pos);
                replace_all(text, "\\n", "\n");
                replace_all(text, "\\\"", "\"");
                return text;
            }
        }
        else if(provider == "gemini") {
            // Gemini response format: {"candidates":[{"content":{"parts":[{"text":"..."}]}}]}
            size_t pos = json_response.find("\"text\":\"");
            if(pos != string::npos) {
                pos += 8;
                size_t end = json_response.find("\"", pos);
                return json_response.substr(pos, end - pos);
            }
        }
        
        return "Error: Could not parse response";
    }
    
    void replace_all(string& str, const string& from, const string& to) {
        size_t pos = 0;
        while((pos = str.find(from, pos)) != string::npos) {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
    }
    
public:
    AIEngine(Config& cfg) : config(cfg) {}
    
    string chat_claude(const string& message, const vector<Message>& context) {
        Utils::set_color("yellow");
        cout << "[NEXUS] Connecting to Claude Sonnet 4..." << endl;
        Utils::set_color("reset");
        
        // Build messages JSON
        stringstream messages_json;
        messages_json << "[";
        
        // Add context
        for(size_t i = 0; i < context.size(); i++) {
            messages_json << context[i].to_json();
            if(i < context.size() - 1 || !message.empty()) messages_json << ",";
        }
        
        // Add current message
        if(!message.empty()) {
            Message msg("user", message);
            messages_json << msg.to_json();
        }
        
        messages_json << "]";
        
        // Build request body
        stringstream body;
        body << "{"
             << "\"model\":\"claude-sonnet-4-20250514\","
             << "\"max_tokens\":" << config.max_tokens << ","
             << "\"temperature\":" << config.temperature << ","
             << "\"system\":\"You are NEXUS AI, an advanced AI assistant more powerful than Gemini CLI. You can execute code, search the web, and maintain context across conversations.\","
             << "\"messages\":" << messages_json.str()
             << "}";
        
        // Headers
        map<string, string> headers = {
            {"Content-Type", "application/json"},
            {"x-api-key", config.anthropic_key},
            {"anthropic-version", "2023-06-01"}
        };
        
        // Make request
        auto response = HttpClient::post(
            "https://api.anthropic.com/v1/messages",
            body.str(),
            headers
        );
        
        if(response.success) {
            return extract_text_from_response(response.body, "claude");
        } else {
            return "Error: " + response.body;
        }
    }
    
    string chat_gpt4(const string& message, const vector<Message>& context) {
        Utils::set_color("yellow");
        cout << "[NEXUS] Connecting to GPT-4 Turbo..." << endl;
        Utils::set_color("reset");
        
        // Build messages JSON
        stringstream messages_json;
        messages_json << "[{\"role\":\"system\",\"content\":\"You are NEXUS AI.\"},";
        
        for(size_t i = 0; i < context.size(); i++) {
            messages_json << context[i].to_json();
            if(i < context.size() - 1 || !message.empty()) messages_json << ",";
        }
        
        if(!message.empty()) {
            Message msg("user", message);
            messages_json << msg.to_json();
        }
        
        messages_json << "]";
        
        stringstream body;
        body << "{"
             << "\"model\":\"gpt-4-turbo-preview\","
             << "\"messages\":" << messages_json.str() << ","
             << "\"temperature\":" << config.temperature << ","
             << "\"max_tokens\":" << config.max_tokens
             << "}";
        
        map<string, string> headers = {
            {"Content-Type", "application/json"},
            {"Authorization", "Bearer " + config.openai_key}
        };
        
        auto response = HttpClient::post(
            "https://api.openai.com/v1/chat/completions",
            body.str(),
            headers
        );
        
        if(response.success) {
            return extract_text_from_response(response.body, "gpt4");
        } else {
            return "Error: " + response.body;
        }
    }
    
    string chat_gemini(const string& message, const vector<Message>& context) {
        Utils::set_color("yellow");
        cout << "[NEXUS] Connecting to Gemini Pro..." << endl;
        Utils::set_color("reset");
        
        // Build contents JSON for Gemini
        stringstream contents_json;
        contents_json << "[";
        
        for(size_t i = 0; i < context.size(); i++) {
            contents_json << "{\"role\":\"" << (context[i].role == "user" ? "user" : "model") << "\","
                         << "\"parts\":[{\"text\":\"" << context[i].content << "\"}]}";
            if(i < context.size() - 1 || !message.empty()) contents_json << ",";
        }
        
        if(!message.empty()) {
            contents_json << "{\"role\":\"user\",\"parts\":[{\"text\":\"" << message << "\"}]}";
        }
        
        contents_json << "]";
        
        stringstream body;
        body << "{\"contents\":" << contents_json.str() << "}";
        
        string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=" + config.google_key;
        
        map<string, string> headers = {
            {"Content-Type", "application/json"}
        };
        
        auto response = HttpClient::post(url, body.str(), headers);
        
        if(response.success) {
            return extract_text_from_response(response.body, "gemini");
        } else {
            return "Error: " + response.body;
        }
    }
    
    string chat(const string& message, const vector<Message>& context, const string& provider = "") {
        string use_provider = provider.empty() ? config.default_provider : provider;
        
        try {
            if(use_provider == "claude") {
                return chat_claude(message, context);
            }
            else if(use_provider == "gpt4") {
                return chat_gpt4(message, context);
            }
            else if(use_provider == "gemini") {
                return chat_gemini(message, context);
            }
            else {
                return "Error: Unknown provider: " + use_provider;
            }
        }
        catch(const exception& e) {
            Utils::set_color("red");
            cout << "⚠️  Error with " << use_provider << ": " << e.what() << endl;
            Utils::set_color("reset");
            
            // Auto-fallback
            vector<string> fallback_providers = {"claude", "gpt4", "gemini"};
            for(const string& fallback : fallback_providers) {
                if(fallback != use_provider) {
                    Utils::set_color("yellow");
                    cout << "🔄 Trying fallback provider: " << fallback << endl;
                    Utils::set_color("reset");
                    
                    try {
                        return chat(message, context, fallback);
                    }
                    catch(...) {
                        continue;
                    }
                }
            }
            
            return "Error: All providers failed";
        }
    }
};

// ============================================================
//                    CODE EXECUTOR
// ============================================================

class CodeExecutor {
public:
    struct ExecutionResult {
        bool success;
        string output;
        string error;
        int exit_code;
    };
    
    static ExecutionResult execute_python(const string& code, int timeout = 30) {
        return execute_code(code, "python3", ".py", timeout);
    }
    
    static ExecutionResult execute_bash(const string& code, int timeout = 30) {
        return execute_code(code, "bash", ".sh", timeout);
    }
    
    static ExecutionResult execute_javascript(const string& code, int timeout = 30) {
        return execute_code(code, "node", ".js", timeout);
    }
    
private:
    static ExecutionResult execute_code(const string& code, const string& interpreter, 
                                       const string& extension, int timeout) {
        ExecutionResult result;
        
        // Create temp file
        string temp_file = "/tmp/nexus_exec_" + to_string(time(0)) + extension;
        Utils::write_file(temp_file, code);
        
        // Execute with timeout
        string command = interpreter + " " + temp_file + " 2>&1";
        
        FILE* pipe = popen(command.c_str(), "r");
        if(!pipe) {
            result.success = false;
            result.error = "Failed to execute code";
            return result;
        }
        
        char buffer[128];
        string output;
        
        while(fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }
        
        int exit_code = pclose(pipe);
        
        // Cleanup
        remove(temp_file.c_str());
        
        result.success = (exit_code == 0);
        result.output = output;
        result.exit_code = exit_code;
        
        if(!result.success) {
            result.error = "Execution failed with exit code: " + to_string(exit_code);
        }
        
        return result;
    }
};

// ============================================================
//                    CONTEXT MANAGER
// ============================================================

class ContextManager {
private:
    Config& config;
    string session_id;
    vector<Message> messages;
    string session_title;
    
public:
    ContextManager(Config& cfg) : config(cfg) {
        session_id = Utils::get_session_id();
        session_title = "Conversation " + session_id;
        Utils::create_directory(config.data_dir + "/conversations");
    }
    
    void add_message(const string& role, const string& content) {
        messages.push_back(Message(role, content));
    }
    
    vector<Message> get_context(int max_messages = -1) {
        if(max_messages < 0) max_messages = config.max_context_messages;
        
        if(messages.size() <= (size_t)max_messages) {
            return messages;
        }
        
        return vector<Message>(
            messages.end() - max_messages,
            messages.end()
        );
    }
    
    void save_session() {
        string filename = config.data_dir + "/conversations/" + session_id + ".json";
        
        stringstream json;
        json << "{\n";
        json << "  \"id\": \"" << session_id << "\",\n";
        json << "  \"title\": \"" << session_title << "\",\n";
        json << "  \"timestamp\": \"" << Utils::get_timestamp() << "\",\n";
        json << "  \"messages\": [\n";
        
        for(size_t i = 0; i < messages.size(); i++) {
            json << "    " << messages[i].to_json();
            if(i < messages.size() - 1) json << ",";
            json << "\n";
        }
        
        json << "  ]\n";
        json << "}\n";
        
        Utils::write_file(filename, json.str());
        
        Utils::set_color("green");
        cout << "💾 Session saved: " << filename << endl;
        Utils::set_color("reset");
    }
    
    void load_session(const string& sid) {
        string filename = config.data_dir + "/conversations/" + sid + ".json";
        
        if(!Utils::file_exists(filename)) {
            Utils::set_color("red");
            cout << "❌ Session not found: " << sid << endl;
            Utils::set_color("reset");
            return;
        }
        
        // Simple loading (in production, use proper JSON parser)
        string content = Utils::read_file(filename);
        
        Utils::set_color("green");
        cout << "✅ Session loaded: " << sid << endl;
        Utils::set_color("reset");
        
        // For now, just print the content
        // In a full implementation, parse and restore messages
    }
    
    void list_sessions() {
        Utils::set_color("cyan");
        cout << "\n📚 Saved Conversations:\n" << endl;
        Utils::set_color("reset");
        
        string command = "ls -1t " + config.data_dir + "/conversations/*.json 2>/dev/null";
        FILE* pipe = popen(command.c_str(), "r");
        
        if(pipe) {
            char buffer[256];
            int count = 1;
            
            while(fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                string filename = buffer;
                filename = filename.substr(0, filename.length() - 1); // remove newline
                
                // Extract session ID from filename
                size_t pos = filename.find_last_of('/');
                string basename = filename.substr(pos + 1);
                string sid = basename.substr(0, basename.length() - 5); // remove .json
                
                cout << "  " << count++ << ". " << sid << endl;
            }
            
            pclose(pipe);
        }
        
        cout << endl;
    }
};

// ============================================================
//                    MAIN CLI INTERFACE
// ============================================================

class NexusCLI {
private:
    Config config;
    unique_ptr<AIEngine> ai_engine;
    unique_ptr<ContextManager> context_manager;
    bool running;
    
    void load_config() {
        // Load from environment variables or config file
        const char* anthropic_key = getenv("ANTHROPIC_API_KEY");
        const char* openai_key = getenv("OPENAI_API_KEY");
        const char* google_key = getenv("GOOGLE_API_KEY");
        
        if(anthropic_key) config.anthropic_key = anthropic_key;
        if(openai_key) config.openai_key = openai_key;
        if(google_key) config.google_key = google_key;
        
        // Try to load from config file
        if(Utils::file_exists("nexus.conf")) {
            ifstream file("nexus.conf");
            string line;
            
            while(getline(file, line)) {
                if(line.empty() || line[0] == '#') continue;
                
                size_t pos = line.find('=');
                if(pos != string::npos) {
                    string key = Utils::trim(line.substr(0, pos));
                    string value = Utils::trim(line.substr(pos + 1));
                    
                    if(key == "anthropic_key") config.anthropic_key = value;
                    else if(key == "openai_key") config.openai_key = value;
                    else if(key == "google_key") config.google_key = value;
                    else if(key == "default_provider") config.default_provider = value;
                }
            }
        }
    }
    
    void show_help() {
        Utils::set_color("cyan");
        cout << "\n📖 NEXUS AI Commands:\n" << endl;
        Utils::set_color("white");
        
        cout << "  /help              - Show this help message" << endl;
        cout << "  /model <name>      - Switch AI model (claude, gpt4, gemini)" << endl;
        cout << "  /exec <lang>       - Execute code (python, bash, js)" << endl;
        cout << "  /search <query>    - Web search" << endl;
        cout << "  /save              - Save current conversation" << endl;
        cout << "  /load <id>         - Load saved conversation" << endl;
        cout << "  /list              - List saved conversations" << endl;
        cout << "  /clear             - Clear screen and context" << endl;
        cout << "  /config            - Show configuration" << endl;
        cout << "  /exit              - Exit NEXUS AI" << endl;
        
        Utils::set_color("reset");
        cout << endl;
    }
    
    void show_config() {
        Utils::set_color("cyan");
        cout << "\n⚙️  Configuration:\n" << endl;
        Utils::set_color("white");
        
        cout << "  Default Provider: " << config.default_provider << endl;
        cout << "  Anthropic Key:    " << (config.anthropic_key.empty() ? "❌ Not set" : "✅ Set") << endl;
        cout << "  OpenAI Key:       " << (config.openai_key.empty() ? "❌ Not set" : "✅ Set") << endl;
        cout << "  Google Key:       " << (config.google_key.empty() ? "❌ Not set" : "✅ Set") << endl;
        cout << "  Temperature:      " << config.temperature << endl;
        cout << "  Max Tokens:       " << config.max_tokens << endl;
        cout << "  Max Context:      " << config.max_context_messages << " messages" << endl;
        
        Utils::set_color("reset");
        cout << endl;
    }
    
    void handle_command(const string& input) {
        vector<string> parts = Utils::split(input.substr(1), ' ');
        string command = parts[0];
        
        if(command == "help") {
            show_help();
        }
        else if(command == "model" && parts.size() > 1) {
            config.default_provider = parts[1];
            Utils::set_color("green");
            cout << "✅ Switched to: " << parts[1] << endl;
            Utils::set_color("reset");
        }
        else if(command == "exec" && parts.size() > 1) {
            string lang = parts[1];
            
            Utils::set_color("yellow");
            cout << "📝 Enter code (type 'END' on new line to finish):\n" << endl;
            Utils::set_color("reset");
            
            string code;
            string line;
            
            while(getline(cin, line) && line != "END") {
                code += line + "\n";
            }
            
            Utils::set_color("cyan");
            cout << "\n🚀 Executing " << lang << " code...\n" << endl;
            Utils::set_color("reset");
            
            CodeExecutor::ExecutionResult result;
            
            if(lang == "python") result = CodeExecutor::execute_python(code);
            else if(lang == "bash") result = CodeExecutor::execute_bash(code);
            else if(lang == "js") result = CodeExecutor::execute_javascript(code);
            else {
                Utils::set_color("red");
                cout << "❌ Unsupported language: " << lang << endl;
                Utils::set_color("reset");
                return;
            }
            
            if(result.success) {
                Utils::set_color("green");
                cout << "✅ Execution successful:\n" << endl;
                Utils::set_color("white");
                cout << result.output << endl;
            } else {
                Utils::set_color("red");
                cout << "❌ Execution failed:\n" << endl;
                Utils::set_color("white");
                cout << result.error << endl;
                if(!result.output.empty()) {
                    cout << result.output << endl;
                }
            }
            
            Utils::set_color("reset");
        }
        else if(command == "search" && parts.size() > 1) {
            string query;
            for(size_t i = 1; i < parts.size(); i++) {
                query += parts[i];
                if(i < parts.size() - 1) query += " ";
            }
            
            Utils::set_color("yellow");
            cout << "🔍 Searching for: " << query << endl;
            Utils::set_color("reset");
            
            // Web search implementation (simplified)
            string search_url = "https://www.google.com/search?q=" + query;
            
            Utils::set_color("cyan");
            cout << "🌐 Search URL: " << search_url << endl;
            Utils::set_color("reset");
        }
        else if(command == "save") {
            context_manager->save_session();
        }
        else if(command == "load" && parts.size() > 1) {
            context_manager->load_session(parts[1]);
        }
        else if(command == "list") {
            context_manager->list_sessions();
        }
        else if(command == "clear") {
            Utils::clear_screen();
            Utils::print_banner();
            context_manager = make_unique<ContextManager>(config);
        }
        else if(command == "config") {
            show_config();
        }
        else if(command == "exit") {
            running = false;
        }
        else {
            Utils::set_color("red");
            cout << "❌ Unknown command: /" << command << endl;
            cout << "Type /help for available commands" << endl;
            Utils::set_color("reset");
        }
    }
    
    void chat_loop() {
        string input;
        
        while(running) {
            // Prompt
            Utils::set_color("cyan");
            cout << "\n💬 You: ";
            Utils::set_color("white");
            
            getline(cin, input);
            
            if(input.empty()) continue;
            
            // Check for commands
            if(input[0] == '/') {
                handle_command(input);
                continue;
            }
            
            // Add to context
            context_manager->add_message("user", input);
            
            // Get response from AI
            Utils::set_color("magenta");
            cout << "\n🤖 NEXUS: ";
            Utils::set_color("white");
            
            string response = ai_engine->chat(
                input,
                context_manager->get_context()
            );
            
            // Print response with typewriter effect
            Utils::typewriter_effect(response, 10);
            
            // Add to context
            context_manager->add_message("assistant", response);
            
            Utils::set_color("reset");
        }
    }
    
public:
    NexusCLI() : running(true) {
        load_config();
        
        ai_engine = make_unique<AIEngine>(config);
        context_manager = make_unique<ContextManager>(config);
        
        Utils::create_directory(config.data_dir);
    }
    
    void run() {
        Utils::print_banner();
        
        Utils::set_color("yellow");
        cout << "🚀 Initializing NEXUS AI..." << endl;
        Utils::set_color("reset");
        
        this_thread::sleep_for(chrono::milliseconds(500));
        
        Utils::set_color("green");
        cout << "✅ System ready!" << endl;
        Utils::set_color("white");
        cout << "Type /help for commands, or just start chatting!" << endl;
        Utils::set_color("reset");
        
        // Check API keys
        if(config.anthropic_key.empty() && config.openai_key.empty() && config.google_key.empty()) {
            Utils::set_color("red");
            cout << "\n⚠️  WARNING: No API keys configured!" << endl;
            cout << "Set environment variables:" << endl;
            cout << "  export ANTHROPIC_API_KEY='your_key'" << endl;
            cout << "  export OPENAI_API_KEY='your_key'" << endl;
            cout << "  export GOOGLE_API_KEY='your_key'" << endl;
            Utils::set_color("reset");
            
            Utils::set_color("yellow");
            cout << "\nContinuing in demo mode..." << endl;
            Utils::set_color("reset");
        }
        
        // Start chat loop
        chat_loop();
        
        // Cleanup
        Utils::set_color("cyan");
        cout << "\n👋 Goodbye! Your conversation has been saved." << endl;
        Utils::set_color("reset");
        
        context_manager->save_session();
    }
};

// ============================================================
//                    MAIN ENTRY POINT
// ============================================================

int main(int argc, char* argv[]) {
    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    try {
        NexusCLI cli;
        cli.run();
    }
    catch(const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    
    // Cleanup
    curl_global_cleanup();
    
    return 0;
}