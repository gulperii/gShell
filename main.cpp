#include <iostream>
#include <vector>
#include <sstream>
#include <iterator>
#include <zconf.h>
#include <wait.h>
#include <map>
#include <queue>
#include <fcntl.h>


//TODO: find kısmını callcommandden önce yap
using namespace std;
map<string, string> LOOKUP;
queue<pair<int, string>> HISTORY;
int counter = 0;

int HISTORY_LIMIT = 15;

void createDict() {

    LOOKUP["listdir"] = "ls";
    LOOKUP["currentpath"] = "pwd";
    LOOKUP["printfile"] = "cat";
    LOOKUP["grep"] = "grep";


}

void pushHistory(queue<pair<int, string>> &history, string command) {
    if (history.size() > HISTORY_LIMIT) {
        history.pop();
    }
    history.push(make_pair(counter, command));
    counter++;

}

void printQueue(queue<pair<int, string>> history) {
    while (!history.empty()) {
        pair<int, string> el = history.front();
        int commandNumber = el.first;
        string command = el.second;
        cout << commandNumber << ' ' << command << "\n";
        history.pop();
    }
}

void searchCharandConvert(vector<string> &tokens, char **ctokens) {
    int len = tokens.size();
    for (int i = 0; i < tokens.size(); i++) {
        string token = tokens[i];
        ctokens[i] = &tokens[i][0];
    }
    ctokens[len] = NULL;
}

vector<string> parseInput(string sentence) {
    istringstream iss(sentence);
    vector<string> tokens{istream_iterator<string> {iss}, istream_iterator<string>{}};
    return tokens;
}

void print(vector<string> &a) {
    for (int i = 0; i < a.size(); i++) {
        cout << a.at(i) << "\n";

    }
}

int callParameterless(string command) {
    pid_t pid, wpid;
    int status;
    char *args[2];
    if (LOOKUP.find(command) == LOOKUP.end()) {
        cout << "Invalid command, not cool :( maybe switch back to WInDoWs ^.^\n";
        return -1;
    }
    args[0] = &LOOKUP[command][0];
    args[1] = NULL;
    pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

void grep(vector<string> query,int i,char **args) {
    int p[2];
    int pid;
    int r;
    int status;



    char *grep[] = {"grep", args[i+2], NULL};

    pipe(p);

    pid = fork();
    if (pid != 0) {
        // Parent: Output is to child via pipe[1]
        dup2(p[0], 0);
        close(p[1]);

        execvp("grep", grep);
        close(p[0]);
        waitpid(pid, &status, WUNTRACED);
        // Change stdout to pipe[1]

    } else {
        // Child: Input is from pipe[0] and output is via stdout.
        dup2(p[1], 1);
        close(p[0]);
        if (query.at(i-1) == "-a"){
            char *ls[] = {"ls","-a",NULL};
            execvp("ls", ls);


        }else{
            char *ls[] = {"ls", NULL};
            execvp("ls", ls);

        }
        exit(EXIT_SUCCESS);

    }

}


int grepWrapper(vector<string> query,int i,char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        grep( query,i,args);


        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

void redirect(vector<string> query,int i,char **args){
    char *newargs[3];
    string fileS = query.at(i - 1);
    string fileD = query.at(i + 1);
    newargs[0] = args[0];
    newargs[1] = &fileS[0];
    newargs[2] = NULL;
    int out = open(&fileD[0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    dup2(out, 1);
    close(out);
    execvp(newargs[0], newargs);
    exit(EXIT_FAILURE);
}

int callWithParameters(vector<string> query) {
    pid_t pid, wpid;
    int status;
    char *args[query.size() + 1];
    searchCharandConvert(query, args);
    if (LOOKUP.find(query[0]) == LOOKUP.end()) {
        cout << "Invalid command :( maybe switch back to GUI ^.^\n";
        return -1;
    }
    args[0] = &LOOKUP[args[0]][0];
   pid = fork();
    if (pid == 0) {
        for (int i = 0; i < query.size(); i++) {
            if (query.at(i) == ">") {
                redirect(query,i,args);
            }
            else if (query.at(i) == "|") {
                grepWrapper(query,i,args);
            }
        }
        execvp(args[0], args);
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;

}


void callCommand(vector<string> &query) {
    if (query.size() == 1) {
        if (query[0] == "footprint") {
            printQueue(HISTORY);

        } else if (query[0] == "exit") {
            exit(EXIT_SUCCESS);
        } else {
            callParameterless(query.at(0));
        }

    } else {
        callWithParameters(query);
    }
}


vector<string> readTerminalInput() {
    string input_line;
    getline(cin, input_line);
    pushHistory(HISTORY, input_line);
    vector<string> i = parseInput(input_line);
    return i;
}

int main() {
    createDict();
    cout << "Welcome on board cool user! GUI is for noobs" << "\n";
    while (true) {
        string username = getenv("USER");
        cout << username << " >>> ";
        vector<string> input = readTerminalInput();
        callCommand(input);
        //grepWrapper();
    }

}
