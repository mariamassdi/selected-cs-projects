#include <unistd.h>
#include <regex>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Commands.h"
#include <sys/syscall.h>
#include <signal.h>
#include <sys/wait.h>
#include "dirent.h"
#include <cstring>     // For strcpy
#include <fcntl.h>     // For open and its flags
#include <sys/types.h> // For data types
#include <sys/stat.h>  // For mode constants
#include <ctime>       // or: #include <time.h>

using namespace std;

bool isValidName(const std::string &name);
bool isRedirectionCommand(const std::string &commandLine, std::string &redirectionType);

const std::string WHITESPACE = " \n\r\t\f\v";
static bool error_printed = false;

#if 0
#define FUNC_ENTRY() \
    cout << __PRETTY_FUNCTION__ << " --> " << endl;
#define FUNC_EXIT() \
    cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else

#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;)
    {
        args[i] = (char *)malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;
    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos)
    {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&')
    {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

SmallShell::SmallShell() : prompt("smash"), pid(-1) {}

SmallShell::~SmallShell() {}

std::string SmallShell::resolve_alias(const std::string &cmd_line)
{
    std::istringstream iss(cmd_line);
    std::string firstWord;
    string s = "";
    if (isRedirectionCommand(cmd_line.c_str(), s))
    {
        firstWord = _trim(cmd_line.substr(0, cmd_line.find_first_of(">")).c_str());
    }
    else if (string(cmd_line).find_first_of("|") != std::string::npos)
    {
        std::string line = cmd_line;
        firstWord = _trim(line.substr(0, line.find_first_of("|")));
    }
    else
    {
        iss >> firstWord;
    }
   if (alias_map.find(firstWord) != alias_map.end())
{
    size_t start = cmd_line.find(firstWord);
    return cmd_line.substr(0, start) + alias_map[firstWord] + cmd_line.substr(start + firstWord.length());
}
return cmd_line;
}

Command *SmallShell::CreateCommand(const char *cmd_line)
{
    if (*cmd_line == '\0')
    {
        return nullptr;
    }
    error_printed = false;
    std::string resolved_cmd = resolve_alias(cmd_line);
    std::string firstWord = _trim(resolved_cmd);
    firstWord = firstWord.substr(0, firstWord.find_first_of(" \n"));

    std::string temp1 = "";
    if (firstWord.compare("alias") == 0)
    {
        aliasCommand *temp = new aliasCommand(resolved_cmd.c_str());
        if (temp->getName() == "-1")

        {
            delete temp;

            if (isRedirectionCommand(string(resolved_cmd.c_str()), temp1))
            {
                return new RedirectionCommand(resolved_cmd.c_str());
            }
            else if (string(resolved_cmd).find_first_of("|") != std::string::npos)
            {
                return new PipeCommand(resolved_cmd.c_str());
            }
            if (!error_printed)
            {
                 std::cerr << "smash error: alias: invalid alias format" << endl;
            }

            error_printed = false;
    
            return nullptr;
        }
        return temp;
    }
    else if (isRedirectionCommand(string(resolved_cmd.c_str()), temp1))
    {
        return new RedirectionCommand(resolved_cmd.c_str());
    }
    else if (string(resolved_cmd).find_first_of("|") != std::string::npos)
    {
        return new PipeCommand(resolved_cmd.c_str());
    }

    else if (firstWord.compare("sysinfo") == 0)
    {
        return new SysInfoCommand(resolved_cmd.c_str());
    }
    //    else if (firstWord.compare("listdir") == 0)
    //    {
    //        ListDirCommand *l = new ListDirCommand(resolved_cmd.c_str(), (string(cmd_line)));
    //        if (l->getPath() == "-1") {
    //            delete l;
    //            return nullptr;
    //        }
    //        return l;
    //    }
    else if (firstWord.compare("whoami") == 0)
    {
        return new WhoAmICommand(resolved_cmd.c_str(), (string(cmd_line)));
    }
    else if (firstWord.compare("usbinfo") == 0)
    {
        return new UsbInfoCommand(resolved_cmd.c_str(), (string(cmd_line)));
    }
    else if (firstWord.compare("pwd") == 0)
    {
        return new GetCurrDirCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("showpid") == 0)
    {
        return new ShowPidCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("chprompt") == 0)
    {
        return new ChangePrompt(resolved_cmd.c_str());
    }
    else if (firstWord.compare("cd") == 0)
    {
        return new ChangeDirCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("jobs") == 0)
    {
        return new JobsCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("fg") == 0)
    {
        return new ForegroundCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("quit") == 0)
    {
        return new QuitCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("kill") == 0)
    {
        return new KillCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("unalias") == 0)
    {
        return new unaliasCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("unsetenv") == 0)
    {
        return new UnSetEnvCommand(resolved_cmd.c_str());
    }
    else if (firstWord.compare("du") == 0)
    {
        return new duCommand(resolved_cmd.c_str(), string(cmd_line));
    }
    else
    {
        return new ExternalCommand(resolved_cmd.c_str(), string(cmd_line));
    }
    return nullptr;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line, "")
{
    std::string resolved_cmd = SmallShell::getInstance().resolve_alias(cmd_line);
    std::istringstream comm(_trim(resolved_cmd).c_str());
    std::string s;
    comm >> this->name;

    if (this->name == "alias")
    {
        std::string rest_of_command;
        std::getline(comm, rest_of_command);
        if (!rest_of_command.empty())
        {
            size_t equals_pos = rest_of_command.find('=');
            if (equals_pos != std::string::npos)
            {
                std::string alias_name = _trim(rest_of_command.substr(0, equals_pos));
                std::string alias_command = _trim(rest_of_command.substr(equals_pos + 1));
                if (alias_command.empty())
                {
                    this->name = "-1";
                     return;
                }

                if (alias_command.back() == '&')
                    alias_command = _trim(alias_command.substr(0, alias_command.find_last_of('&')));
                if ((alias_command.front() == '"' && alias_command.back() == '"') || (alias_command.front() == '\'' && alias_command.back() == '\''))
                {
                    alias_command = alias_command.substr(1, alias_command.size() - 2);
                }
                else
                {
                    this->name = "-1";
                }

                if (isValidName(alias_name) && this->name != "-1")
                {
                    args.push_back(alias_name);
                    args.push_back(alias_command);
                }
                else
                {
                    this->name = "-1";
                }
            }
        }
        else
        {
        this->name="-2";
        }
    }
    else
    {
        for (; comm >> s;)
        {
            args.push_back(s);
        }
    }
}

const vector<string> &BuiltInCommand::getArgs() const
{
    return args;
}

string &BuiltInCommand::getName()
{
    return name;
}

#define BUFFER_SIZE 4096

static bool check(const char *var_name)
{
    int fd = open("/proc/self/environ", O_RDONLY);
    if (fd < 0)
    {
        std::cerr << "smash error: open failed" << std::endl;
        return false;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    size_t envar_len = strlen(var_name);
    std::string leftover;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        std::string data = leftover + std::string(buffer, bytes_read);
        size_t pos = 0;

        while (pos < data.size())
        {
            size_t end = data.find('\0', pos);
            if (end == std::string::npos)
            {
                // לא קיבלנו מחרוזת שלמה — נשמור אותה לסיבוב הבא
                leftover = data.substr(pos);
                break;
            }

            std::string entry = data.substr(pos, end - pos);
            if (entry.compare(0, envar_len, var_name) == 0 && entry[envar_len] == '=')
            {
                close(fd);
                return true;
            }
            pos = end + 1;
        }
    }

    close(fd);
    return false;
}

UnSetEnvCommand::UnSetEnvCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void UnSetEnvCommand::execute()
{
    extern char **__environ;

    const vector<std::string> &args = this->getArgs();
    if (args.size() < 1)
    {
        std::cerr << "smash error: unsetenv: not enough arguments" << std::endl;
        return;
    }

    for (size_t i = 0; i < args.size(); ++i)
    {
        const std::string &var = args[i];
        if (!check(var.c_str()))
        {
            std::cerr << "smash error: unsetenv: " << var << " does not exist" << std::endl;
            return;
        }

        size_t var_len = strlen(var.c_str());
        for (int i = 0; __environ[i] != nullptr; ++i)
        {
            if (strncmp(__environ[i], var.c_str(), var_len) == 0 && __environ[i][var_len] == '=')
            {
                for (int j = i; __environ[j] != nullptr; ++j)
                {
                    __environ[j] = __environ[j + 1];
                }
                break;
            }
        }
    }
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand((cmd_line)) {}

void GetCurrDirCommand::execute()
{
    char path[COMMAND_MAX_LENGTH + 1];
    if (getcwd(path, COMMAND_MAX_LENGTH + 1) != nullptr)
    {
        std::cout << string(path) << endl;
    }
    else
    {
        perror("smash error: getcwd failed");
        return;
    }
}

std::string ChangeDirCommand::Old_Path;

ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand((cmd_line)) {}

void ChangeDirCommand::execute()
{
    const vector<std::string> &args = this->getArgs();
    if (args.size() > 1)
    {
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }

    char current[COMMAND_MAX_LENGTH + 1];
    if (getcwd(current, COMMAND_MAX_LENGTH + 1) == nullptr)
    {
        perror("smash error: getcwd failed");
        return;
    }

    std::string current_dir(current);
    string new_Dir;

    if (args.front() == "-")
    {
        if (Old_Path.size() == 0)
        {
            std::cerr << "smash error: cd: OLDPWD not set\n";
            return;
        }
        else
        {
            new_Dir = Old_Path;
        }
    }
    else if (args.front() == "..")
    {
        std::string parentDir;
        size_t lastSlash = current_dir.find_last_of("/");
        if (lastSlash == std::string::npos || (lastSlash == 0 && current_dir.length() == 1))
        {
            parentDir = current_dir;
        }
        else if (lastSlash == 0)
        {
            parentDir = "/";
        }
        else
        {
            parentDir = current_dir.substr(0, lastSlash);
        }
        new_Dir = parentDir;
    }
    else
    {
        new_Dir = args.front();
    }

    if (chdir(new_Dir.c_str()))
    {
        perror("smash error: chdir failed");
        return;
    }
    else
    {
        Old_Path = current_dir;
    }
}

QuitCommand::QuitCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void QuitCommand::execute()
{
    JobsList &jobs = SmallShell::getInstance().getBackGroundJobs();
    const vector<std::string> args = getArgs();
    if (args.size() > 0)
    {
        if (args.front() == "kill")
        {
            std::cout << "smash: sending SIGKILL signal to " << jobs.getNumOfJobs() << " jobs:" << endl;
            if (jobs.getNumOfJobs() > 0)
            {
                jobs.killAllJobs();
            }
        }
    }
    delete this;
    exit(0);
}

KillCommand::KillCommand(const char *cmd_line) : BuiltInCommand(cmd_line), flag(0)
{
    const vector<std::string> &args = getArgs();
    try
    {
        m_job_id = std::stoi(args.at(1));
        if (m_job_id < 1)
        {
            throw exception();
        }
        if (args.size() != 2)
        {
            throw exception();
        }
        m_signal_number = std::stoi(args.front());
        if (m_signal_number > 0)
        {
            std::cerr << "smash error: kill: invalid arguments" << endl;
        }
        m_signal_number *= -1;
    }
    catch (...)
    {
        std::cerr << "smash error: kill: invalid arguments\n";
        flag = 1;
    }
}

void KillCommand::execute()
{
    JobsList::JobEntry *job = SmallShell::getInstance().getBackGroundJobs().getJobById((m_job_id));
    if (m_signal_number < 0)
    {
        return;
    }
    if (job == nullptr && flag == 0)
    {
        std::cerr << "smash error: kill: job-id " << m_job_id << " does not exist\n";
        return;
    }

    if (job != nullptr)
    {
        if (flag == 0)
        {
            cout << "signal number " << m_signal_number << " was sent to pid " << job->getPID() << endl;
        }

        if (flag != 1 && kill(job->getPID(), m_signal_number) != 0)
        {
            perror("smash error: kill failed");
            return;
        }
    }
}

unaliasCommand::unaliasCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

bool isValidName(const std::string &name)
{
    const std::unordered_set<std::string> reservedKeywords = {
        "quit", "showpid", "cd", "chprompt", "pwd", "jobs", "fg", "kill", "alias", "unalias", "whoami", "du", "unsetenv", "sysinfo", "usbinfo"};

    if (reservedKeywords.find(name) != reservedKeywords.end())
    {
        cerr << "smash error: alias: " << name << " already exists or is a reserved command" << endl;
        error_printed=true;
        return false;
    }

    std::regex nameRegex("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(name, nameRegex))
    {
        cerr << "smash error: alias: invalid alias format" << endl;
        error_printed=true;
        return false;
    }

    return true;
}

void unaliasCommand::execute()
{
    if (getArgs().size() < 1)
    {
        std::cerr << "smash error: unalias: not enough arguments" << std::endl;
        return;
    }
    try{
    for (unsigned int i = 0; i < getArgs().size(); ++i)
    {
        SmallShell::getInstance().removeAlias(getArgs()[i]);
    }
    }
    catch(...)
    {
        return ;
    }
}

aliasCommand::aliasCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void aliasCommand::execute()
{
    
    if (getArgs().empty() && getName()=="-2")
    {
        SmallShell::getInstance().printAliases();
        return;
    }

    if (getArgs().size() == 1)
    {
        std::cerr << "smash error: alias: invalid alias format" << std::endl;
        return;
    }

    if (getArgs().size() == 2)
    {
        std::string aliasName = getArgs()[0];
        std::string aliasValue = getArgs()[1];

        if (SmallShell::getInstance().addAlias(aliasName, aliasValue) == -1)
        {
            std::cerr << "smash error: alias: " << aliasName << " already exists or is a reserved command" << std::endl;
            return;
        }

        return;
    }

    std::cerr << "smash error: alias: invalid alias format" << std::endl;
}

int SmallShell::addAlias(const std::string &alias_name, const std::string &command)
{
    if (alias_map.find(alias_name) == alias_map.end())
    {
        alias_map[alias_name] = command;
        alias_vector.push_back(pair<string, string>(alias_name, command));
    }
    else
    {
        return -1;
    }
    return 0;
}

void SmallShell::removeAlias(const string &args)
{
    string name = args;
    if (alias_map.find(args) != alias_map.end())
    {
        string com = alias_map[args];
        alias_map.erase(name);
        for (std::vector<pair<string, string>>::iterator v = alias_vector.begin(); v != alias_vector.end(); v++)
        {
            if (v->first == name && v->second == com)
            {
                alias_vector.erase(v);
                return;
            }
        }
    }
    else
    {
        std::cerr << "smash error: unalias: " << name << " alias does not exist" << std::endl;
         throw 1;
        return;
    }
}

void SmallShell::printAliases()
{
    for (const auto &alias : alias_vector)
    {
        std::cout << alias.first << "='" << alias.second << "'" << std::endl;
    }
}

void SmallShell::executeCommand(const char *cmd_line)
{
    BackGroundJobs.removeFinishedJobs();
    Command *cmd = CreateCommand(cmd_line);
    if (cmd)
    {
        cmd->execute();
        delete cmd;
    }
    setPID(-1);
}

void SmallShell::setPID(pid_t PID)
{
    this->pid = PID;
}

pid_t SmallShell::getPID()
{
    return pid;
}

const string &SmallShell::GetPrompt()
{
    return this->prompt;
}

JobsList &SmallShell::getBackGroundJobs()
{
    return this->BackGroundJobs;
}

void SmallShell::change_prompt(const string &prompt)
{
    this->prompt = prompt;
}

JobsList::JobEntry::JobEntry(string &cmd_line, pid_t PID, int job_Id) : cmd_line(cmd_line), PID(PID), job_Id(job_Id) {}

JobsList::JobEntry::~JobEntry() {}

string &JobsList::JobEntry::get_cmd_line()
{
    return cmd_line;
}

pid_t JobsList::JobEntry::getPID()
{
    return PID;
}

int JobsList::JobEntry::getJobId()
{
    return job_Id;
}

JobsList::JobsList() : num_of_jobs(0) {}

JobsList::~JobsList() {}

void JobsList::addJob(Command *cmd, pid_t PID)
{
    if (jobs.empty())
    {
        num_of_jobs = 0;
    }
    jobs.push_back(JobEntry(cmd->getOriginal(), PID, num_of_jobs + 1));
    num_of_jobs++;
}

void JobsList::printJobsList()
{
    for (JobEntry &job : jobs)
    {
        cout << "[" << job.getJobId() << "] " << job.get_cmd_line() << endl;
    }
}

void JobsList::killAllJobs()
{
    for (JobEntry &job : jobs)
    {
        cout << job.getPID() << ": " << job.get_cmd_line() << endl;
        if (kill(job.getPID(), SIGKILL) != 0)
        {
            perror("smash error: kill failed");
        }
    }
    jobs.clear();
}

void JobsList::removeFinishedJobs()
{
    vector<int> forRemove;
    for (JobEntry &job : jobs)
    {
        int res = waitpid(job.getPID(), nullptr, WNOHANG);
        if (res == -1 || res == job.getPID())
        {
            forRemove.push_back(job.getJobId());
        }
    }
    for (int jobId : forRemove)
    {
        JobsList::removeJobById(jobId);
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
    for (JobEntry &job : jobs)
    {
        if (job.getJobId() == jobId)
        {
            return &job;
        }
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId)
{
    for (std::vector<JobEntry>::iterator job = jobs.begin(); job != jobs.end(); job++)
    {
        if (job->getJobId() == jobId)
        {
            if (num_of_jobs == jobId)
            {
                num_of_jobs--;
            }
            jobs.erase(job);
            return;
        }
    }
}

JobsList::JobEntry *JobsList::getLastJob()
{
    return &jobs.back();
}

int JobsList::getNumOfJobs()
{
    return jobs.size();
}

void JobsList::setNumOfJobs(int numOfJobs)
{
    this->num_of_jobs = numOfJobs;
}

vector<JobsList::JobEntry> &JobsList::getJobsList()
{
    return jobs;
}

JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void JobsCommand::execute()
{
    SmallShell::getInstance().getBackGroundJobs().printJobsList();
}

ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line), jobId(-100)
{
    JobsList &jobs = SmallShell::getInstance().getBackGroundJobs();
    if (BuiltInCommand::getArgs().empty() && jobs.getNumOfJobs() > 0)
    {
        this->jobId = jobs.getLastJob()->getJobId();
    }
    else if (BuiltInCommand::getArgs().size() == 1)
    {
        try
        {
            this->jobId = stoi(BuiltInCommand::getArgs().front());
        }
        catch (...)
        {
            this->jobId = -100;
        }
    }
}

void ForegroundCommand::execute()
{
    JobsList &jobs = SmallShell::getInstance().getBackGroundJobs();
    if (jobs.getNumOfJobs() == 0 && BuiltInCommand::getArgs().size() == 0 && jobId < 0)
    {
        cerr << "smash error: fg: jobs list is empty" << endl;
        return;
    }

    if (BuiltInCommand::getArgs().size() > 1 || jobId < 0)
    {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    if (jobs.getJobById(jobId) == nullptr)
    {
        cerr << "smash error: fg: job-id " << jobId << " does not exist" << endl;
        return;
    }

    JobsList::JobEntry *job = jobs.getJobById(jobId);
    pid_t PID = job->getPID();

    SmallShell::getInstance().setPID(PID);

    cout << job->get_cmd_line() << " " << PID << endl;

    jobs.removeJobById(jobId);

    if (waitpid(PID, nullptr, WUNTRACED) == -1)
    {
        perror("smash error: waitpid failed");
        return;
    }

    SmallShell::getInstance().setPID(-1);
}

bool isRedirectionCommand(const std::string &commandLine, std::string &redirectionType)
{
    size_t posSingle = commandLine.find('>');
    size_t posDouble = commandLine.find(">>");
    if (posSingle == std::string::npos)
    {
        return false;
    }
    if (posDouble != std::string::npos && posDouble == posSingle)
    {
        redirectionType = ">>";
    }
    else
    {
        redirectionType = ">";
    }
    return true;
}

RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line, " ")
{
    isRedirectionCommand(cmd_line, redirectionType);
    std::string cmd_str(cmd_line);
    command = _trim(cmd_str.substr(0, cmd_str.find_first_of(">")).c_str());
    path = _trim(cmd_str.substr(cmd_str.find_last_of(">") + 1));
}

void RedirectionCommand::execute()
{
    if (this->isBackground() == true)
    {
        path = _trim(path.substr(0, path.find_last_of('&')));
    }
    int new_fd;
    int dupped_fd = dup(1);
    if (dupped_fd == -1)
    {
        perror("smash error: dup failed");
        return;
    }
    if (close(1))
    {
        perror("smash error: close failed");
        return;
    }
    int writing_flag = O_APPEND;
    if (redirectionType == ">")
    {
        writing_flag = O_TRUNC;
    }
    new_fd = open(path.c_str(), O_WRONLY | O_CREAT | writing_flag, 0666);
    if (new_fd == -1)
    {
        perror("smash error: open failed");
        if (dup2(dupped_fd, 1) == -1)
        {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(dupped_fd) == -1)
        {
            perror("smash error: close failed");
            return;
        }
        return;
    }
    SmallShell::getInstance().executeCommand(command.c_str());
    if (close(new_fd) == -1)
    {
        perror("smash error: close failed");
        exit(0);
    }
    if (dup2(dupped_fd, 1) == -1)
    {
        perror("smash error: dup2 failed");
        return;
    }
    if (close(dupped_fd) == -1)
    {
        perror("smash error: close failed");
        return;
    }
}

ExternalCommand::ExternalCommand(const char *cmdLine, string origianl) : Command(cmdLine, origianl)
{
    if (std::string(cmdLine).find_first_of("*?") != std::string::npos)
    {
        complexity = Complexity::Complex;
    }
    else
    {
        complexity = Complexity::Simple;
    }
}

void ExternalCommand::execute()
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("smash error: fork failed ");
        return;
    }

    if (pid != 0)
    {
        if (!isBackground())
        {
            SmallShell::getInstance().setPID(pid);
            if (waitpid(pid, nullptr, WUNTRACED) == -1)
            {
                perror("smash error: waitpid failed");
            }
            SmallShell::getInstance().setPID(-1);
        }
        else
        {
            SmallShell::getInstance().getBackGroundJobs().addJob(this, pid);
        }
    }
    else
    {
        if (setpgrp() == -1)
        {
            perror("smash error: setpgrp failed");
            return;
        }

        if (complexity == Complexity::Simple)
        {
            char *args[COMMAND_MAX_ARGS + 1] = {0};
            char trimmed_cmd_line[COMMAND_MAX_LENGTH + 1];
            strcpy(trimmed_cmd_line, getLine().c_str());
            _removeBackgroundSign(trimmed_cmd_line);
            _parseCommandLine(_trim(std::string(trimmed_cmd_line)).c_str(), args);
            if (execvp(args[0], args) == -1)
            {
                perror("smash error: execvp failed");
            }
            exit(0);
        }
        else
        {
            std::string command_line_str = _trim(Command::remove_background_sign(getLine().c_str()));
            const char *command_line = command_line_str.c_str();

            if (execlp("/bin/bash", "/bin/bash", "-c", command_line, nullptr) != 0)
            {
                perror("smash error: execlp failed");
                return;
            }
        }
    }
}

ExternalCommand::~ExternalCommand() {}

duCommand::duCommand(const char *cmd_line, string origianl) : Command(cmd_line, origianl)
{
    std::string resolved_cmd = SmallShell::getInstance().resolve_alias(cmd_line);
    std::istringstream comm(_trim(resolved_cmd).c_str());
    for (std::string s; comm >> s;)
    {
        args.push_back(s);
    }
}

long duCommand::calculateDirSize(const std::string &path)
{
    struct stat statbuf;
    if (lstat(path.c_str(), &statbuf) != 0)
    {
        return 0; // cannot stat
    }

    // Skip symbolic links entirely
    if (S_ISLNK(statbuf.st_mode))
    {
        return 0;
    }

    // For regular files, just return the actual file size in bytes
    if (S_ISREG(statbuf.st_mode))
    {
        return statbuf.st_blocks;
    }

    // For directories, include the directory entry size and its contents
    long total_size = statbuf.st_blocks;

    if (S_ISDIR(statbuf.st_mode))
    {
        int dirfd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
        if (dirfd == -1)
        {
            perror("smash error: open failed");
            return total_size;
        }

        const size_t bufSize = 20000;
        char buf[bufSize];
        ssize_t nread;

        // iterate entries via getdents64 syscall
        while ((nread = syscall(SYS_getdents64, dirfd, buf, bufSize)) > 0)
        {
            size_t bpos = 0;
            while (bpos < (size_t)nread)
            {
                struct dirent64 *d = (struct dirent64 *)(buf + bpos);
                const char *name = d->d_name;
                // skip "." and ".."
                if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
                {
                    std::string child = path;
                    if (child.back() != '/')
                        child += '/';
                    child += name;
                    total_size += calculateDirSize(child);
                }
                bpos += d->d_reclen;
            }
        }

        close(dirfd);
    }
    return total_size;
}
void duCommand::execute()
{
    // Check if there are too many arguments
    if (args.size() > 2)
    { // args[0] is "du", args[1] would be the path
        std::cerr << "smash error: du: too many arguments" << std::endl;
        return;
    }

    std::string path;
    if (args.size() == 1)
    {
        // No path provided, use current directory
        path = ".";
    }
    else
    {
        path = args[1];
    }

    // Check if the directory exists
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0 || !S_ISDIR(statbuf.st_mode))
    {
        std::cerr << "smash error: du: directory " << path << " does not exist" << std::endl;
        return;
    }

    // Calculate the directory size
    long blocks = calculateDirSize(path); // סכום כל הבלוקים
    long size_KB = (blocks + 1) / 2;      // עיגול למעלה אחרי הסכימה

    std::cout << "Total disk usage: " << size_KB << " KB" << std::endl;
}

WhoAmICommand::WhoAmICommand(const char *cmd_line, string origianl) : Command(cmd_line, origianl) {}

void WhoAmICommand::execute()
{
    uid_t current_uid = getuid();
    gid_t current_gid = getgid();

    int fd = open("/etc/passwd", O_RDONLY);

    if (fd == -1)
    {
        perror("smash error: open failed");
        return;
    }

    char buffer[4096]; // Increased buffer size to accommodate longer lines
    ssize_t bytes_read;

    // Read /etc/passwd line by line
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0'; // Null-terminate the string

        char *line = buffer;
        while (line && *line != '\0')
        {
            char *next_line = strchr(line, '\n');
            if (next_line)
            {
                *next_line = '\0'; // Null-terminate the current line
                next_line++;       // Move to the next line
            }

            // Parse the line to extract the username and UID
            char *username = strtok(line, ":");
            strtok(nullptr, ":");
            char *uid_str = strtok(nullptr, ":");
            strtok(nullptr, ":");
            strtok(nullptr, ":");
            char *home_dir = strtok(nullptr, ":");
            strtok(nullptr, ":");

            // Convert the UID to an integer
            if (username && uid_str)
            {
                unsigned int uid = atoi(uid_str);
                if (uid == current_uid)
                {
                    // Found the matching UID, output the username and home directory
                    std::cout << username << std::endl;
                    std::cout << current_uid << std::endl;
                    std::cout << current_gid << std::endl;
                    std::cout << home_dir << std::endl;
                    if (close(fd) == -1)
                    {
                        perror("smash error: close failed");
                    }
                    return;
                }
            }

            line = next_line; // Move to the next line
        }
    }

    if (close(fd) == -1)
    {
        perror("smash error: close failed");
    }
}

PipeCommand::PipeCommand(const char *cmd_line) : Command(cmd_line, "")
{
    std::string s(cmd_line);
    unsigned int index_of_line = s.find_first_of("|");
    int index_of_amp = s.find_first_of("&");

    if (index_of_line != std::string::npos)
    {
        if (index_of_amp != -1)
        {
            pipeType = PipeType::Error;
            cmd2 = s.substr(index_of_amp + 1, s.size() - index_of_amp);
        }
        else
        {
            pipeType = PipeType::Standard;
            cmd2 = s.substr(index_of_line + 1, s.size() - index_of_line);
        }

        cmd1 = s.substr(0, index_of_line);
        cmd1 = _trim(cmd1);
        cmd2 = _trim(cmd2);
    }
}

void PipeCommand::execute()
{
    int files[2];
    if (pipe(files) == -1)
    {
        perror("smash error: pipe failed");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == -1)
    {
        perror("smash error: fork failed");
        if (close(files[READ]) == -1)
        {
            perror("smash error: close failed");
            return;
        }
        if (close(files[WRITE]) == -1)
        {
            perror("smash error: close failed");
            return;
        }
        return;
    }

    if (pid1 == 0)
    {
        if (setpgrp() == -1)
        {
            perror("smash error: setpgrp failed");
            return;
        }

        if (pipeType == PipeType::Error)
        {
            if (dup2(files[WRITE], STDERR_FILENO) == -1)
            {
                perror("smash error: dup2 failed");
                return;
            }
        }
        else
        {
            if (dup2(files[WRITE], STDOUT_FILENO) == -1)
            {
                perror("smash error: dup2 failed");
                return;
            }
        }

        if (close(files[READ]) == -1)
        {
            perror("smash error: close failed");
            return;
        }

        if (close(files[WRITE]) == -1)
        {
            perror("smash error: close failed");
            return;
        }

        SmallShell::getInstance().executeCommand(cmd1.c_str());
        exit(0);
    }
    else
    {
        if (close(files[WRITE]) == -1)
        {
            perror("smash error: close failed");
            return;
        }
    }

    pid_t pid2 = fork();
    if (pid2 == -1)
    {
        perror("smash error: fork failed");
        if (close(files[READ]) == -1)
        {
            perror("smash error: close failed");
            return;
        }
        return;
    }

    if (pid2 == 0)
    {
        if (setpgrp() == -1)
        {
            perror("smash error: setpgrp failed");
            return;
        }

        if (dup2(files[READ], STDIN_FILENO) == -1)
        {
            perror("smash error: dup2 failed");
            return;
        }

        if (close(files[READ]) == -1)
        {
            perror("smash error: close failed");
            return;
        }

        SmallShell::getInstance().executeCommand(cmd2.c_str());
        exit(0);
    }
    else
    {
        if (close(files[READ]) == -1)
        {
            perror("smash error: close failed");
            return;
        }
    }

    if (waitpid(pid1, nullptr, WUNTRACED) == -1)
    {
        perror("smash error: waitpid failed");
    }

    if (waitpid(pid2, nullptr, WUNTRACED) == -1)
    {
        perror("smash error: waitpid failed");
    }
}

ChangePrompt::ChangePrompt(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void ChangePrompt::execute()
{
    SmallShell::getInstance().change_prompt((getArgs().size() == 0) ? "smash" : getArgs().front());
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void ShowPidCommand::execute()
{
    pid_t pid = getpid();
    if (pid == -1)
    {
        perror("smash error: getpid failed");
        return;
    }
    cout << "smash pid is " << pid << endl;
}

std::string Command::remove_background_sign(const char *cmd_line) const
{
    char *cmd_line_copy = new char[strlen(cmd_line) + 1];
    strcpy(cmd_line_copy, cmd_line);
    _removeBackgroundSign(cmd_line_copy);
    std::string new_cmdline(cmd_line_copy);
    delete[] cmd_line_copy;
    return new_cmdline;
}

string &Command::getOriginal()
{
    return original_cmd;
}

Command::Command(const char *cmd_line, string resolve) : groundType((_isBackgroundComamnd(cmd_line)) ? GroundType::Background : GroundType::Foreground),
                                                         cmd_line(cmd_line), original_cmd(resolve) {}

string &Command::getLine()
{
    return this->cmd_line;
}

SysInfoCommand::SysInfoCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

static std::string Get_FirstLine(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        return "open_fail";
    }

    char buffer[256];
    ssize_t line = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (line <= 0)
    {
        return "read_fail";
    }

    buffer[line] = '\0';

    std::string st(buffer);
    size_t pos = st.find('\n');
    if (pos != std::string::npos)
    {
        st = st.substr(0, pos);
    }

    return st;
}
static bool Failure_check(const std::string &ch)
{
    if (ch == "open_fail")
    {
        std::cerr << "smash error: open failed" << std::endl;
        return true; // there WAS a failure
    }
    if (ch == "read_fail")
    {
        std::cerr << "smash error: read failed" << std::endl;
        return true;
    }
    return false;
}
static bool isMissingField(const std::string &line)
{
    return line == "open_fail" || line == "read_fail";
}
static std::string ReadFile(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return "open_fail";

    char buf[4096];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n <= 0)
        return "read_fail";

    buf[n] = '\0';
    return std::string(buf);
}
void SysInfoCommand::execute()
{
    // find the System
    std::string System = Get_FirstLine("/proc/sys/kernel/ostype");
    if (Failure_check(System))
        return;

    // Hostname
    std::string Hostname = Get_FirstLine("/proc/sys/kernel/hostname");
    if (Failure_check(Hostname))
        return;
    //  Kernel
    std::string Kernel = Get_FirstLine("/proc/sys/kernel/osrelease");
    if (Failure_check(Kernel))
        return;
    //  Architecture -as explaind at the piazza
    std::string Architecture = "x86_64";
    // for the boot time file
    std::string BTime_file = ReadFile("/proc/stat");
    if (Failure_check(BTime_file))
        return;

    std::string Btime_file(BTime_file);

    std::string BT = "btime ";
    size_t pos = Btime_file.find(BT);
    if (pos == std::string::npos)
    {
        std::cerr << "smash error: btime not found" << std::endl;
        return;
    }

    pos += BT.length();

    //  extract the number until end of line 
    size_t endpos = Btime_file.find('\n', pos);
    std::string num;
    if (endpos == std::string::npos)
        num = Btime_file.substr(pos); 
    else
        num = Btime_file.substr(pos, endpos - pos);

    long long boot_epoch = atoll(num.c_str());///converting

    time_t bt = (time_t)boot_epoch;///converting

    struct tm *info = localtime(&bt);
    if (!info)
    {
        std::cerr << "smash error: localtime failed" << std::endl;
        return;
    }
    char result[64];
    strftime(result, sizeof(result), "%Y-%m-%d %H:%M:%S", info);

    // Print with required format
    std::cout << "System: " << System << std::endl;
    std::cout << "Hostname: " << Hostname << std::endl;
    std::cout << "Kernel: " << Kernel << std::endl;
    std::cout << "Architecture: " << Architecture << std::endl;
    std::cout << "Boot Time: " << result << std::endl;
}

UsbInfoCommand::UsbInfoCommand(const char *cmd_line, string original)
    : Command(cmd_line, original) {}

struct D_info
{
    int devnum = -100;             
    std::string vendor = "N/A";  
    std::string product = "N/A"; 
    std::string manufacturer = "N/A";
    std::string product_name = "N/A";
    std::string power = "N/A"; 
};

static bool extract_first_integer(const std::string &s, int &value)
{
    try
    {
        value = std::stoi(s);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

static bool compare_dinfo_by_devnum(const D_info &a, const D_info &b)
{
    return a.devnum < b.devnum;
}


void UsbInfoCommand::execute()
{

    int dirfd = open("/sys/bus/usb/devices", O_RDONLY | O_DIRECTORY);
    if (dirfd < 0)
    {
        perror("smash error: open failed");
        return;
    }

    const std::size_t BUF_SIZE = 8192;
    char buf[BUF_SIZE];
    ssize_t nread;

    std::vector<D_info> devices;

    while ((nread = syscall(SYS_getdents64, dirfd, buf, BUF_SIZE)) > 0)
    {
        std::size_t bpos = 0;
        while (bpos < (std::size_t)nread)
        {
            struct dirent64 *d = (struct dirent64 *)(buf + bpos);
            const char *name = d->d_name;

            if (name[0] == '.' &&
                (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
            {
                bpos += d->d_reclen;
                continue;
            }

            if (strchr(name, ':') != NULL)
            {
                bpos += d->d_reclen;
                continue;
            }

             std::string devicePath = std::string("/sys/bus/usb/devices") + "/" + name;
            D_info info; 
            std::string line;
            bool hasDevnum = false;
            bool hasVendor = false;
            bool hasProduct = false;

            line = _trim(Get_FirstLine((devicePath + "/devnum").c_str()));
            if (!isMissingField(line))
            {
                int devnumValue;
                if (extract_first_integer(line, devnumValue))
                {
                    info.devnum = devnumValue;
                    hasDevnum = true;
                }
            }
            if (!hasDevnum)
            {
                bpos += d->d_reclen;
                continue;
            }

            // idVendor

            line = _trim(Get_FirstLine(((devicePath + "/idVendor").c_str())));
            if (!isMissingField(line))

            {
                info.vendor = _trim(line);
                hasVendor = true;
            }
          
          
            // idProduct
            line = _trim(Get_FirstLine(((devicePath + "/idProduct").c_str())));
            if (!isMissingField(line))
            {
                info.product = _trim(line);
                hasProduct = true;
            }
            if (!hasVendor || !hasProduct)
            {
                bpos += d->d_reclen;
                continue;
            }

            line = _trim(Get_FirstLine(((devicePath + "/manufacturer").c_str())));
            if (!isMissingField(line))
            {
                std::string m = _trim(line);
                if (!m.empty())
                {
                    info.manufacturer = m;
                }
            }

            line = _trim(Get_FirstLine(((devicePath + "/product").c_str())));
            if (!isMissingField(line))
            {
                std::string p = _trim(line);
                if (!p.empty())
                {
                    info.product_name = p;
                }
            }

            
            line = _trim(Get_FirstLine(((devicePath + "/bMaxPower").c_str())));
            if (!isMissingField(line))

            {
                int mA;
                if (extract_first_integer(line, mA))
                {
                    std::ostringstream oss;
                    oss << mA; 
                    info.power = oss.str();
                }
            }

            devices.push_back(info);
            bpos += d->d_reclen;
        }
    }

   
    if (close(dirfd) == -1)
    {
        perror("smash error: close failed");
    }

    if (devices.empty())
    {
        std::cerr << "smash error: usbinfo: no USB devices found" << std::endl;
        return;
    }

    std::sort(devices.begin(), devices.end(), compare_dinfo_by_devnum);

    
    // Device <devnum>: ID <vendor>:<product> <manufacturer> <product_name> MaxPower: <power>mA
    for (std::size_t i = 0; i < devices.size(); ++i)
    {
        std::cout << "Device " << devices[i].devnum
                  << ": ID " << devices[i].vendor << ":" << devices[i].product << " "
                  << devices[i].manufacturer << " " << devices[i].product_name
                  << " MaxPower: " << devices[i].power << "mA"
                  << std::endl;
    }
}
