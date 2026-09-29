#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <unordered_set>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <cstring>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using namespace std;

enum class GroundType {
    Foreground,
    Background
};

enum PIPE {
    READ = 0,
    WRITE = 1
};

class Command {
    GroundType groundType; // should come before the command line
    string cmd_line;
    string original_cmd;

public:
    Command(const char *cmd_line, string original);
    virtual ~Command() {}
    bool isBackground() const { return groundType == GroundType::Background; }
    virtual void execute() = 0;
    string &getLine();
    std::string remove_background_sign(const char *cmd_line) const;
    string &getOriginal();
};

class BuiltInCommand : public Command {
private:
    string name;
    vector<string> args;

public:
    BuiltInCommand(const char *cmd_line);
    virtual ~BuiltInCommand() {}
    const vector<string> &getArgs() const;
    string &getName();
};

//BUILT_IN_COMMANDS
class ChangePrompt : public BuiltInCommand {
public:
    ChangePrompt(const char *cmd_line);
    virtual ~ChangePrompt() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand {
public:
    UnSetEnvCommand(const char *cmd_line);

    virtual ~UnSetEnvCommand() {
    }

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
    static std::string Old_Path;

public:
    ChangeDirCommand(const char *cmd_line);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class JobsCommand : public BuiltInCommand {
    public:
        JobsCommand(const char *cmd_line);
        virtual ~JobsCommand() {}
        void execute() override;
    };

class ForegroundCommand : public BuiltInCommand {
    int jobId;

public:
    ForegroundCommand(const char *cmd_line);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class QuitCommand : public BuiltInCommand {
public:
    QuitCommand(const char *cmd_line);
    virtual ~QuitCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    int m_signal_number;
    int m_job_id;
    bool flag;

public:
    KillCommand(const char *cmd_line);
    virtual ~KillCommand() {}
    void execute() override;
};

class aliasCommand : public BuiltInCommand {
public:
    aliasCommand(const char *cmd_line);
    virtual ~aliasCommand() {}
    void execute() override;
};

class unaliasCommand : public BuiltInCommand {
public:
    unaliasCommand(const char *cmd_line);
    virtual ~unaliasCommand() {}
    void execute() override;
};
class SysInfoCommand : public BuiltInCommand {
    public:
    SysInfoCommand(const char *cmd_line);
     virtual ~SysInfoCommand() {}
    void execute();

};
//EXTERNAL_COMMANDS
class ExternalCommand : public Command {
    enum class Complexity {
        Simple,
        Complex
    };

    Complexity complexity;
    pid_t m_pid;

public:
    ExternalCommand(const char *cmd_line, string original);
    virtual ~ExternalCommand();
    void execute() override;
};

//SPECIAL_COMMANDS
class RedirectionCommand : public Command {
    string command;
    string path;
    string redirectionType;

public:
    explicit RedirectionCommand(const char *cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
};


class PipeCommand : public Command {
public:
    enum class PipeType {
        Standard,
        Error
    };

    PipeCommand(const char *cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;

private:
    PipeType pipeType;
    std::string cmd1;
    std::string cmd2;
};

class duCommand : public Command {
    long calculateDirSize(const std::string &path);
    std::vector<std::string> args;
public:
    duCommand(const char *cmd_line,string original);
    virtual ~duCommand() {}
    void execute() override;
};

class UsbInfoCommand : public Command {
public:
    UsbInfoCommand(const char *cmd_line,string original) ;
    virtual ~UsbInfoCommand() {}
    void execute() override;
};

class WhoAmICommand : public Command {
    public:
        WhoAmICommand(const char *cmd_line, string original);
        virtual ~WhoAmICommand() {}
        void execute() override;
    };


// JobsList and related classes
class JobsList {
public:
    class JobEntry {
    private:
        string cmd_line;
        pid_t PID;
        int job_Id;

    public:
        JobEntry(string &cmd_line, pid_t PID, int job_Id);
        ~JobEntry();
        string &get_cmd_line();
        pid_t getPID();
        int getJobId();
    };

    JobsList();
    ~JobsList();

    void addJob(Command *cmd, pid_t PID);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry *getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry *getLastJob();
    int getNumOfJobs();
    void setNumOfJobs(int numOfJobs);
    vector<JobEntry> &getJobsList();

private:
    int num_of_jobs;
    vector<JobEntry> jobs;
};

class SmallShell {
private:
    std::map<std::string, std::string> alias_map;
    vector<pair<string, string>> alias_vector;
    std::string prompt;
    pid_t pid;
    JobsList BackGroundJobs;

    SmallShell();

public:
    Command *CreateCommand(const char *cmd_line);
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator

    static SmallShell &getInstance() {
        static SmallShell instance; // Guaranteed to be destroyed.
        return instance;
    }

    void printAliases();
    ~SmallShell();
    int addAlias(const std::string &alias_name, const std::string &command);
    void removeAlias(const string &args);
    std::string resolve_alias(const std::string &cmd_line);
    void executeCommand(const char *cmd_line);
    void change_prompt(const string &prompt);
    const string &GetPrompt();
    pid_t getPID();
    void setPID(pid_t PID);
    JobsList &getBackGroundJobs();
};



#endif // SMASH_COMMAND_H_
