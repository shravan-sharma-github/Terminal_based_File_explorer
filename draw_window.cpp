#include "header.h"

using namespace std;
pair<int, int> cursor_at;

int getWindowSize(int &rows, int &cols)
{
    winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        return -1;
    }
    else
    {
        cols = ws.ws_col;
        rows = ws.ws_row;
        return 0;
    }
}

void get_dir_content(char path[], vector<pair<string, struct stat>> &contents )
{
    DIR *dir;
    struct dirent *entry;
    struct stat stats;
    if ((dir = opendir(path)) == NULL)
        die("opendir()");
    else
    {
        while ((entry = readdir(dir)) != NULL)
        {
            // if(only_file_folder && (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,".."))){
            // //then not add this entry in contents
            //     continue;
            // }
            //updated path in temp
            string temp = string(path) + '/' + string(entry->d_name);
            if (stat(temp.c_str(), &stats) == 0)
            {       
                contents.push_back({entry->d_name, stats});
            }
            else
            {
                die("stat()");
                //cout << "Unable to get file properties.\n";
                //cout<<"Please check whether "<<path<<" file exists.\n";
                //perror("Unable to open file");
            }
        }
        closedir(dir);
    }
}

void move_cursor_to(int row, int col ,pair<int,int> *cursor = NULL)
{
    if(cursor){
        cursor->first = row;
        cursor->second = col;
    }
    cout << "\x1b[" << row << ";" << col << "H"; //set cursor at (row,col)
    fflush(stdout);
}

void clear_screen()
{
    move_cursor_to(0,0);
    write(STDOUT_FILENO, "\x1b[2J", 4); //clear screen from corsur position till bottom
}
//note after calling it move cursor at previouse position
void draw_footer(int window_height,int window_width , pair<int,int> *cursor){
    string cmd1 = "  copy <source_file(s)> <destination_directory>";
    string cmd2 = "\tdelete_dir <dir_path>";
    string cmd3 = "  move <source_file(s)> <destination_directory>";
    string cmd4 = "\tdelete_file <file_path>";
    string cmd5 = "  create_file <file_name> <destination_path>   ";
    string cmd6 = "\trename <old_filename> <new_filename>";
    string cmd7 = "  create_dir <dir_name> <destination_path>      ";
    string cmd8 = "\tsearch <file_name/directory_name>";
    string cmd9 = "\tgoto <location>";
    
    int required_lines=3;//for print msg and all
    if(cursor != NULL){
        required_lines +=5;
        if(cmd1.size() + cmd2.size()+4>= window_width){
            required_lines++;
        }
        cursor->first++;
        if(cmd3.size() + cmd4.size() + 4 >= window_width){
            required_lines++;
        }
        cursor->first++;
        if(cmd5.size() + cmd6.size()+4>= window_width){
            required_lines++;
        }
        cursor->first++;
        if(cmd7.size() + cmd7.size() +cmd9.size() + 4 >= window_width){
            required_lines++;
        }
    }
    
    move_cursor_to(window_height -required_lines,1,cursor);
    
        
    

    //draw commands
    
    cout<<BLUE
        <<"Commands : "<<endl
        <<cmd1<<", "
        <<cmd2<<endl
        <<cmd3<<", "
        <<cmd4<<endl
        <<cmd5<<", "
        <<cmd6<<endl
        <<cmd7<<", "
        <<cmd8<<", "
        <<cmd9<<", "
       
    
    <<NORMAL<<endl;

    //move cursor
    
    cursor->first+=5;//for write commands
    
    if(cmd1.size() + cmd2.size()+4>= window_width){
        cursor->first++;
    }
    
    if(cmd3.size() + cmd4.size() + 4 >= window_width){
        cursor->first++;
    }
    
    if(cmd5.size() + cmd6.size()+4>= window_width){
        cursor->first++;
    }
    
    if(cmd7.size() + cmd7.size() +cmd9.size() + 4 >= window_width){
        cursor->first++;
    }
    

    int x = 0;
    while (x++ <window_width/2 - 8)cout<<"=";
    cout<<" comand mode ";
    x = 0;
    while (x++ <window_width/2 - 8)cout<<"=";
    cout<<endl;
    cursor->first++;
    clearLine();
    string command_text = "Type command : ";
    cout<<command_text;
    
    cursor->second = command_text.size()+1;
    
    // move_cursor_to(0,0);
}
//give text like: "<alert_name> : <message>" , color codes allowed :- RED, GREEN, BLUE ,otherwise NORMAL
void alert(int row , string text, pair<int,int> final_cursor_position ,string color = NORMAL){
    move_cursor_to(row,1);
    clearLine();
    cout<<color<<text<<NORMAL;
    move_cursor_to(final_cursor_position.first,final_cursor_position.second);
}