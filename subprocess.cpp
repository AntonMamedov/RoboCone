#include "subprocess.h"

SubProcess::SubProcess(std::function<void()> func){
    int pipe_in[2];
    int pipe_out[2];
    pipe(pipe_in);
    pipe(pipe_out);
    if (fork() == 0)
    {
        close(pipe_in[1]);
        close(pipe_out[0]);
        dup2(pipe_in[0], 0);
        dup2(pipe_out[1], 1);
        func();

    }
    close(pipe_in[0]);
    close(pipe_out[1]);
    fdin = dup(pipe_in[1]);
    fdout = dup(pipe_out[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
}


SubProcess::SubProcess(std::string com){
    int pipe_in[2];
    int pipe_out[2];
    pipe(pipe_in);
    pipe(pipe_out);
    if (fork() == 0)
    {
        close(pipe_in[1]);
        close(pipe_out[0]);
        dup2(pipe_in[0], 0);
        dup2(pipe_out[1], 1);
        int size = 2;
        for (int i = 0; i < com.length(); i++)
        {
            if (com[i] == ' ')
                size++;
        }
        std::vector<char*> arg;
        comparse(com, arg);
        if (execvp(arg.data()[0], arg.data()) < 0){
            _exit(0);
        }
    }
    close(pipe_in[0]);
    close(pipe_out[1]);
    fdin = dup(pipe_in[1]);
    fdout = dup(pipe_out[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
}


SubProcess::~SubProcess(){
    close(fdin);
    close(fdout);
}


void SubProcess::comparse(std::string& com, std::vector<char*>& wordList)
{
    char *s = new char[com.size() + 1];
    strcpy(s, com.c_str());
    char *p = strtok(s, " ");
    wordList.push_back(p);
    while (p != nullptr) {
        p = strtok(nullptr, " ");
        wordList.push_back(p);
    }
}



int SubProcess::Receive(char *buf){
    return read(fdout, buf, 1024);
}

bool SubProcess::Send(const char *buf, int size){
     if (write(fdin, buf, size) == size){
        return true;
    }
    return false;
}

int SubProcess::GetInDescriptor(){
    return fdout;
}
