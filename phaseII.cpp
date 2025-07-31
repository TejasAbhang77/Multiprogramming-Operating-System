#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

class PCB{
    public:
        int jobID, TTL, TLL;
        int TTC,LLC;
};

class VM{
private:

    fstream infile;
    ofstream outfile;
    char Memory[300][4], buffer[40], IR[4], R[4];
    bool C;
    int IC;
    int SI,PI,TI;
    int allocatedPages[30];
    int pageTableINDEX;
    int PTR, PTE, RA, VA, PageFaultValid, pageNo, frame;
    bool Terminate;

    PCB pcb;

    void init(){

        fill(buffer, buffer + sizeof(buffer), '\0');
        fill(&Memory[0][0], &Memory[0][0] + sizeof(Memory), '\0');
        fill(IR, IR + sizeof(IR), '\0');
        fill(R, R + sizeof(R), '\0');
        C = true;
        IC = 0;
        SI = PI = TI = 0;

        pcb.jobID = pcb.TLL = pcb.TTL = pcb.TTC = pcb.LLC = 0;
        PTR=PTE=pageNo=-1;
        fill(allocatedPages, allocatedPages + sizeof(allocatedPages), 0);
        pageTableINDEX = 0;
        Terminate = false;
    }

    void resetBuffer(){
        fill( buffer, buffer + sizeof(buffer), '\0');
    }

    int ALLOCATE(){
        int pageNo;
        bool check=true;
        while(check){
            pageNo = (rand() % 30);
            if(allocatedPages[pageNo]==0){  
                allocatedPages[pageNo] = 1;
                check=false;
            }
        }
        return pageNo;
    }

    int ADDRESSMAP(int VA){
        if (VA<0 || VA>=100) {
            PI = 2; 
            MOS();
            return -1;
        }
        PTE = PTR + (VA / 10);
        if (Memory[PTE][2] == '*' && Memory[PTE][3] == '*') {
            PI = 3;
            MOS();
            return -2;
        }
        string frameString;
        frameString += Memory[PTE][2];
        frameString += Memory[PTE][3];
        frame = stoi(frameString);
        RA = frame*10 + (VA%10);
        return RA;
    }

    void LOAD(){
        if (infile.is_open()){
            string s;

            while (getline(infile, s)){
                if(s.empty()){
                    return;
                }
                if (s[0] == '$' && s[1] == 'A' && s[2] == 'M' && s[3] == 'J'){

                    init();
                    cout << "\n ----- New Job started -----\n";
                    pcb.jobID = (s[4] - '0') * 1000 + (s[5] - '0') * 100 + (s[6] - '0') * 10 + (s[7] - '0');
                    pcb.TTL = (s[8] - '0') * 1000 + (s[9] - '0') * 100 + (s[10] - '0') * 10 + (s[11] - '0');
                    pcb.TLL = (s[12] - '0') * 1000 + (s[13] - '0') * 100 + (s[14] - '0') * 10 + (s[15] - '0');

                    PTR = ALLOCATE()*10;

                    for(int i=PTR;i<PTR+10;i++){
                        for(int j=0;j<4;j++){
                            Memory[i][j]='*';
                        }
                    }

                    cout << "\nAllocated Page for Page Table: " << PTR / 10 << "\n";
                    cout << "jobID: " << pcb.jobID << "\nTTL: " << pcb.TTL << "\nTLL: " << pcb.TLL << "\n";
                }

                else if (s[0] == '$' && s[1] == 'D' && s[2] == 'T' && s[3] == 'A'){
                    cout << "\nData card loding...\n";
                    resetBuffer();
                    STARTEXE();
                }

                else if (s[0] == '$' && s[1] == 'E' && s[2] == 'N' && s[3] == 'D'){
                        cout<<("\n");
                    for(int i=0;i<300;i++){
                        if(i==PTR){
                            cout<<" ----- Page Table -----\n";
                        }
                        if(i==PTR+10){
                            cout<<"  -------- End -------\n";
                        }
                        cout<<"M["<<i<<"] : ";

                        for(int j=0;j<4;j++){
                            cout<<Memory[i][j]<<" ";
                        }
                        cout<<'\n';
                    }
                }

              else {
                  resetBuffer();
                  pageNo = ALLOCATE();
                  Memory[PTR + pageTableINDEX][2] = (pageNo / 10) + '0';
                  Memory[PTR + pageTableINDEX][3] = (pageNo % 10) + '0';
                  pageTableINDEX++;
                  cout << "\nProgram Card loading...";

                  int baseaddress = pageNo * 10;
                  int wordsLoaded = 0;
                  int charPos = 0;

                  for (int i = 0; i < s.size() && wordsLoaded < 10; i++) {
                      Memory[baseaddress][charPos] = s[i];
                      charPos++;

                      if (s[i] == 'H') {
                          for (int j = charPos; j < 4; j++) {
                              Memory[baseaddress][j] = ' ';
                          }
                          charPos = 4;
                      }

                      if (charPos >= 4) {
                          charPos = 0;
                          baseaddress++;
                          wordsLoaded++;
                      }
                  }

                  if (charPos > 0) {
                      for (int j = charPos; j < 4; j++) {
                          Memory[baseaddress][j] = ' ';
                      }
                      baseaddress++;
                  }
              }
            }
            infile.close();
        }
    }

    void READ(){

        string s;
        getline(infile, s);

        if(s.empty()){
            return ;
        }

        if(s[0] == '$' && s[1] == 'E' && s[2] == 'N' && s[3] == 'D'){
            TERMINATE(1);
            return;
        }

        PTE = PTR + (VA / 10);

        if(Memory[PTE][2] == '*' && Memory[PTE][3] == '*'){
            pageNo = ALLOCATE();
            Memory[PTE][2] = (pageNo / 10) + '0';
            Memory[PTE][3] = (pageNo % 10) + '0';
            cout << "Allocated page " << pageNo << " for address " << VA << endl;
        }

        for(int i=0; i<s.size() && i<40; i++) {
            buffer[i] = s[i];
        }

        string frameString;
        frameString += Memory[PTE][2];
        frameString += Memory[PTE][3];
        frame = stoi(frameString);
        RA = frame*10 + (VA % 10);

        int buff = 0;
        int memoryPTR = RA;
        int endPTR = RA + 10;

        while(buff < 40 && buffer[buff] != '\0' && memoryPTR < endPTR) {
            for(int i = 0; i < 4; i++) {
                if(buffer[buff] != '\0') {
                    Memory[memoryPTR][i] = buffer[buff];
                    buff++;
                } else {
                    Memory[memoryPTR][i] = ' ';
                }
            }
            memoryPTR++;
        }
        resetBuffer();
        SI = 0;
    }

    void WRITE(){
        if (Terminate) return;

      if(pcb.LLC>=pcb.TLL){
          TERMINATE(2);
          return;
      }

        pcb.LLC++;

        outfile.open("./output.txt",ios::app);
        bool writing = true;

        for(int i=RA; i<RA+10; i++){
            for(int j=0; j<4; j++){
                if( Memory[i][j] != '\0'){
                    outfile<<Memory[i][j];
                }
                else{
                    writing = false;
                    break;
                }
            }

            if(!writing){
                break;
            }
        }
        SI=0;
        outfile<<"\n";
        outfile.close();
    }

    void TERMINATE(int EM ){
        Terminate = true;
        outfile.open("./output.txt",ios::app);

        switch(EM){
            case 0:
                outfile<<"Program terminated normally.\n";
                break;
            case 1:
                outfile<<"Program terminated abnormally. OUT OF DATA ERROR\n";
                break;
            case 2:
                outfile<<"Program terminated abnormally. LINE LIMIT EXCEEDED ERROR\n";
                break;
            case 3:
                outfile<<"Program terminated abnormally. TIME LIMIT EXCEEDED ERROR\n";
                break;
            case 4:
                outfile<<"Program terminated abnormally. OPERATION CODE ERROR\n";
                break;
            case 5:
                outfile<<"Program terminated abnormally. OPERAND ERROR\n";
                break;
            case 6:
                outfile<<"Program terminated abnormally. INVALID PAGE FAULT\n";
                break;
            case 7:
                outfile<<"Program terminated abnormally. TIME LIMIT EXCEEDED AND OPERATION CODE ERROR\n";
                break;
            case 8:
                outfile<<"Program terminated abnormally. TIME LIMIT EXCEEDED AND OPERAND CODE ERROR\n";
                break;
        }

        outfile<<"JobID : "<<pcb.jobID<<" ";
        outfile<<"IC : "<<IC<<" ";
        outfile<<"IR : ";

        for(int i=0; i<4; i++){
            outfile<<IR[i];
        }

        outfile<<" ";
        outfile<<"SI : "<<SI<<" ";
        outfile<<"PI : "<<PI<<" ";
        outfile<<"TI : "<<TI<<" ";
        outfile<<"TTC : "<<pcb.TTC<<" ";
        outfile<<"LLC : "<<pcb.LLC<<" ";
        outfile<<"TTL : "<<pcb.TTL<<" ";
        outfile<<"TLL : "<<pcb.TLL<<" ";
        outfile<<"\n\n\n";

        SI=PI=TI=0;
        outfile.close();
    }

    void STARTEXE(){
        IC = 0;
        EXECUTEUSERPROGRAM();
    }

void MOS(){
    if(TI==0){
        if(SI==1) READ();
        else if(SI==2) WRITE();
        else if(SI==3) TERMINATE(0);
        else if(PI==1) TERMINATE(4);
        else if(PI==2) TERMINATE(5);
        else if(PI==3){
            if(PageFaultValid == 1){
                pageNo = ALLOCATE();
                Memory[PTE][2] = (pageNo / 10) + '0';
                Memory[PTE][3] = (pageNo % 10) + '0';
                pageTableINDEX++;
                PI = 0;
            }
            else{
                TERMINATE(6);
                return;
            }
        }
    }

    else if(TI==2){
        if(SI==1) TERMINATE(3);
        else if(SI==2) {
            WRITE();
            TERMINATE(3);
        }
        else if(SI==3) TERMINATE(0);
        else if(PI==1) TERMINATE(7);
        else if(PI==2) TERMINATE(8);
        else if(PI==3) TERMINATE(3);
    }
}

void EXECUTEUSERPROGRAM() {
    while (!Terminate) {
        RA = ADDRESSMAP(IC);
        if (PI != 0) return;

        for (int i = 0; i < 4; i++) {
            IR[i] = Memory[RA][i];
        }
        IC++;

        if (IR[0] == 'G' && IR[1] == 'D') {
            PageFaultValid = 1;
            if (!isdigit(IR[2]) || !isdigit(IR[3])) {
                PI = 2;
                MOS();
            } else {
                VA = ((IR[2] - '0') * 10 + (IR[3] - '0'));
                RA = ADDRESSMAP(VA);
                if (PI == 3) {
                    MOS();
                    IC--;
                } else {
                    SIMULATION();
                    SI = 1;
                    MOS();
                   SIMULATION();
                }
            }
        }
        else if (IR[0] == 'P' && IR[1] == 'D') {
            PageFaultValid = 0;
            if (!isdigit(IR[2]) || !isdigit(IR[3])) {
                PI = 2;
                MOS();
            } else {
                VA = ((IR[2] - '0') * 10 + (IR[3] - '0'));
                RA = ADDRESSMAP(VA);
                if (PI == 3) {
                    MOS();
                } else {
                    SI = 2;
                    MOS();
                    SIMULATION();
                }
            }
        }

        else if (IR[0] == 'H') {
            SIMULATION();
            SI = 3;
            MOS();
        }

        else if (IR[0] == 'L' && IR[1] == 'R') {
            PageFaultValid = 0;
            if (!isdigit(IR[2]) || !isdigit(IR[3])) {
                PI = 2;
                MOS();
            } else {
                VA = ((IR[2] - '0') * 10 + (IR[3] - '0'));
                RA = ADDRESSMAP(VA);
                if (PI == 3) {
                    MOS();
                } else {
                    SIMULATION();
                    for (int i = 0; i < 4; i++) {
                        R[i] = Memory[RA][i];
                    }
                }
            }
        }

        else if (IR[0] == 'S' && IR[1] == 'R') {
            PageFaultValid = 1;
            if (!isdigit(IR[2]) || !isdigit(IR[3])) {
                PI = 2;
                MOS();
            } else {
                VA = ((IR[2] - '0') * 10 + (IR[3] - '0'));
                RA = ADDRESSMAP(VA);
                if (PI == 3) {
                    MOS();
                    IC--;
                } else {
                    SIMULATION();
                    for (int i = 0; i < 4; i++) {
                        Memory[RA][i] = R[i];
                    }
                    SIMULATION();
                }
            }
        }

        else if (IR[0] == 'C' && IR[1] == 'R') {
            PageFaultValid = 0;
            if (!isdigit(IR[2]) || !isdigit(IR[3])) {
                PI = 2;
                MOS();
            } else {
                VA = ((IR[2] - '0') * 10 + (IR[3] - '0'));
                RA = ADDRESSMAP(VA);
                if (PI == 3) {
                    MOS();
                } else {
                    SIMULATION();
                    C = true;
                    for (int i = 0; i < 4; i++) {
                        if (Memory[RA][i] != R[i]) {
                            C = false;
                            break;
                        }
                    }
                }
            }
        }

        else if (IR[0] == 'B' && IR[1] == 'T') {
            PageFaultValid = 0;
            if (!isdigit(IR[2]) || !isdigit(IR[3])) {
                PI = 2;
                MOS();
            } else {
                SIMULATION();
                if (C) {
                    IC = ((IR[2] - '0') * 10 + (IR[3] - '0'));
                }
            }
        }

        else { 
            PI = 1;
          SI=0;
            MOS();
        }
    }
}

    void SIMULATION(){
        pcb.TTC += 1;
        if(pcb.TTC >= pcb.TTL){
            TI=2;
            MOS();
        }
    }

public:
    VM(){
        infile.open("input2.txt", ios::in);
        init();
        LOAD();
    }
};

int main(){
    VM v;
    return 0;
}