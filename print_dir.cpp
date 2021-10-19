#include "header.h"

using namespace std;
string months[] = {" Jan "," Feb "," Mar "," Apr "," May "," Jun "," Jul "," Aug "," Sep "," Oct "," Nov "," Dec "};
string getGroup(gid_t uid)
{
    struct group *pws;
    pws = getgrgid(uid);
    return pws->gr_name;
}
string getUser(uid_t uid)
{
    struct passwd *pws;
    pws = getpwuid(uid);
    return pws->pw_name;
}
void printFileProperties(struct stat stats){
    
    cout<<"# ";
    if(S_ISDIR(stats.st_mode)){
        cout<<"d";
    }else{
        cout<<"-";
    }
    // File permissions
    printf((stats.st_mode & S_IRUSR)? "r":"-");
    printf((stats.st_mode & S_IWUSR)? "w":"-");
    printf((stats.st_mode & S_IXUSR)? "x":"-");
    printf(" ");
    printf((stats.st_mode & S_IRGRP)? "r":"-");
    printf((stats.st_mode & S_IWGRP)? "w":"-");
    printf((stats.st_mode & S_IXGRP)? "x":"-");
    printf(" ");
    printf((stats.st_mode & S_IROTH)? "r":"-");
    printf((stats.st_mode & S_IWOTH)? "w":"-");
    printf((stats.st_mode & S_IXOTH)? "x":"-");
    
    //permissions 
    cout<<"\t"<<getUser(stats.st_uid);
    cout<<" "<<getGroup(stats.st_gid);

    //time of modification in local time
    struct tm *dt =localtime(&(stats.st_ctime));
    string month = months[dt->tm_mon];
    cout<<"\t"<< month// dt.tm_year + 1900
        <<setw(2)<< dt->tm_mday <<" "                                
        <<(dt->tm_hour< 10 ? ("0"+to_string(dt->tm_hour)) : to_string(dt->tm_hour))<<":"
        <<(dt->tm_min < 10 ? ("0"+to_string(dt->tm_min)) : to_string(dt->tm_min)) ;
    
    


    // File size
    int file_size = stats.st_size;
    if(file_size/1073741824 > 1)    cout<<setw(5)<<file_size/1073741824<<" GB";
    else if(file_size/1048576 > 1)  cout<<setw(5)<<file_size/1048576<<" MB";
    else if(file_size/1024 > 1)     cout<<setw(5)<<file_size/1024<<" KB";
    else cout<<setw(5)<<file_size<<" B ";
}
//it prints directory content,current_dir is directory related to folder where app started 
void print_directory_content(string current_dir, vector<pair<string, struct stat>> contents,int start_index , int rows){
    if(rows == 0)return;
    while(rows--){
        if(start_index >= contents.size())break;
        auto it = contents[start_index++];
        clearLine();
        printFileProperties(it.second);cout<<"  "<<it.first<<endl;
    }
    cout<<endl<<" current directory : "<<current_dir<<endl;
} 

