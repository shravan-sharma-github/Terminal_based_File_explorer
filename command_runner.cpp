#include "header.h"
void get_dir_content(char path[], vector<pair<string, struct stat>> &contents );
using namespace std;
void create_dir(string path);

//source = source_file_path including file name , same for destination
void copy_file(string source, string destination)
{
    struct stat source_file_stats, destination_file_stats;
    char buff[1024];
    int byte_read;
    //copy source and destination to char array
    char c_source[source.size()];
    char c_destination[destination.size()];
    strcpy(c_source, source.c_str());
    strcpy(c_destination, destination.c_str());

    //open files

    int from = open(c_source, O_RDONLY);
    int to = open(c_destination, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    //read from source and write to destination file

    for (byte_read = read(from, buff, sizeof(buff)); byte_read > 0; byte_read = read(from, buff, sizeof(buff)))
    {
        write(to, buff, byte_read);
    }
    if (byte_read == -1)
    {
        string msg = source + " file read()";
        die(msg.c_str());
    }

    //find stats
    if (stat(c_source, &source_file_stats) < 0)
    {
        die("stat()");
    }
    if (stat(c_destination, &destination_file_stats) < 0)
    {
        die("stat()");
    }

    //change permissions as source file
    if (chown(c_destination, source_file_stats.st_uid, source_file_stats.st_gid) != 0)
    {
        die("chown(): ");
    }
    if (chmod(c_destination, source_file_stats.st_mode) != 0)
    {
        die("chmod(): ");
    }
}

void copy_recursively(string source_dir,string destination_path){
    vector<pair<string, struct stat>> contents;
    char c_dir[source_dir.size()+2];
    strcpy(c_dir,source_dir.c_str());
    get_dir_content(c_dir,contents);
    for(auto it:contents){   
        if(!(it.first == "." || it.first == "..")){
            if(S_ISDIR(it.second.st_mode)){//it is directory so first create dir in destination then copy content recursively
                
                string dest = destination_path + '/' + it.first;
                create_dir(dest);
                copy_recursively(source_dir +'/'+ it.first,dest);
            }else{//it is file so copy it  
                copy_file(source_dir +'/'+ it.first,destination_path + '/' + it.first);
            }
        }
    }
}
//source_files are with their absolute path
void copy_command(vector<string> source_files, string destination_path)
{
    struct stat stats;
    for (auto it : source_files)
    {
        if (stat(it.c_str(), &stats) == 0)
        {   int f = it.find_last_of("/\\");
            string name = it.substr(f+1,it.size());
            if(S_ISDIR(stats.st_mode)){
                
                string dest = destination_path + '/'+ name;
                create_dir(dest);
                copy_recursively(it,dest);
            }else{
                copy_file(it,destination_path + '/' + name);
            }
        }
        else
        {
            die("stat()");
        }        
    }
}
void create_file(string file)
{
    char c_file[file.size()];
    strcpy(c_file, file.c_str());
    if (open(c_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IXOTH) < 0)
    {
        die("create_file(): ");
    }
}
void create_file_command(string destination_path, vector<string> file_names)
{
    for (string name : file_names)
    {
        create_file(destination_path + "/" + name);
    }
}
void create_dir(string path)
{
    char c_dir[path.size()];
    strcpy(c_dir, path.c_str());
    if (mkdir(c_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IXOTH) != 0)
    {
        die("create_dir(): ");
    }
}
void create_dir_command(string destination_path, vector<string> dir_names)
{
    for (string dir_name : dir_names)
    {
        create_dir(destination_path + "/" + dir_name);
    }
}
//path is absolute path of file to delete
void delete_file_command(string path){
    if (remove(path.c_str()) != 0)
        die("remove(): ");
}
bool search_in_this_folder(string name , vector<pair<string,struct stat>> contents){
    for(auto it:contents){
        if(it.first == name)
            return true;
    }
    return false;
}
bool search_command(string file_or_folder_name,char * current_dir, vector<pair<string, struct stat>> contents){
    if(search_in_this_folder(file_or_folder_name,contents)){
        return true;
    }    
    for(auto it:contents){   
        if(!(it.first == "." || it.first == "..")){
            if(S_ISDIR(it.second.st_mode)){                
                vector<pair<string, struct stat>> cont;
                char new_dir[(string(current_dir) + '/' + it.first).size()+2];
                strcpy(new_dir,(string(current_dir) + '/' + it.first).c_str());
                get_dir_content(new_dir,cont);
                if(search_command(file_or_folder_name,new_dir,cont)){
                    return true;
                }
            }
        }
    }
   return false;
}
void delete_dir_command(char * current_dir,int cur_dir_name_size){
    vector<pair<string, struct stat>> contents;
    get_dir_content(current_dir,contents);
       
    for(auto it:contents){   
        if(!(it.first == "." || it.first == "..")){
            int size = cur_dir_name_size + it.first.size()+2 ;//+2 for safer side
            char file[size];
            strcpy(file,(string(current_dir)+"/"+it.first).c_str()); 

            if(S_ISDIR(it.second.st_mode)){//it is directory so remove it's content                
                delete_dir_command(file,size);
            }else{//it is file so remove it 
                if (remove(file) != 0)
                    die("remove(): "); 
            }
        }
    }
    //content of dir deleted, now delete dir
    if (rmdir(current_dir) != 0)
        die("rmdir() : ");
}
//old_files are with absolute path
void move_command(vector<string> old_files , string destination_dir){
    for(auto file:old_files){
        //extract name of file/folder
        int f = file.find_last_of("/\\");
        string name = file.substr(f+1,file.size());

        if (rename(file.c_str(), (destination_dir+"/"+name).c_str()) != 0)
        {
            if(errno == EXDEV){//means files are on different file system so need copy delete
                struct stat stats;
                if (stat(file.c_str(), &stats) == 0)
                {   
                    if(S_ISDIR(stats.st_mode)){//file to move is dir
                        
                        string dest = destination_dir + '/'+ name;
                        create_dir(dest);
                        copy_recursively(file,dest);
                        //now delete dir
                        char c_file[file.size()+2];
                        strcpy(c_file,file.c_str());
                        delete_dir_command(c_file,file.size()+2);
                    }else{
                        string dest = destination_dir + '/'+ name;
                        copy_file(file,dest);
                        //now delete file
                        delete_file_command(file);
                    }
                }
                else
                {
                    die("stat()");
                }   
            }else{
                die("move_command(): ");
            }
        }
    }
}