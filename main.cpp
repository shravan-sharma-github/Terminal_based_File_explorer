#include "all_header.h"

using namespace std;

/*============================= Global variables ========================================*/
//pair<int, int> cursor_at; ---------->global variable at draw_window.cpp
//struct termios orig_termios;-------->global variable at change_mode.cpp
//bool row_mode ---------------------->global variable at change_mode.cpp
int max_line_to_print; //it is min(row_of_window / 3, contents.size() - start_index)
int start_index;       //it is index from which contents are displayed
char home_dir[4096];//assuming it root also
char current_dir[4096];
bool normal_mode ;
vector<pair<string, struct stat>> contents; //current dir contents,first = file/folder_name , second = stat_object
vector<string> right_stack;                 //use only push_back() , back() , pop_back() and clear()
stack<string> left_stack;                   //contains priviously visited paths
string typed_command_till_now;

void die(const char *s)
{
  cout << "\x1b[2J"
       << "\x1b[H" << endl; // to clear screen and move corsur to (0,0)
  // write(STDOUT_FILENO, "\x1b[2J", 4);
  // write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}
void refresh_attributes_and_screen(int sig)
{
  int row, col;
  if (getWindowSize(row, col) == -1)
    die("getWindowSize()");

  getcwd(current_dir, 4096);              //get current working directory , it set current_dir with current working directory
  contents.clear();                       //previous content deleted
  get_dir_content(current_dir, contents); //contents updated

  max_line_to_print = min(row / 3, (int)contents.size());
  //update cursor position and start_index also
  start_index = 0;
  if(cursor_at.first >max_line_to_print){
    cursor_at.first = max_line_to_print;
  }

  clear_screen();
  string dir = "/home" +string(current_dir).substr(string(home_dir).size());
  print_directory_content(dir, contents, start_index, max_line_to_print);
  pair<int,int> final_pos;
  draw_footer(row, col,&final_pos);
  if(normal_mode)
    move_cursor_to(cursor_at.first, cursor_at.second);
  else
    cout<<typed_command_till_now;
}
void init()
{
  enableRawMode();
  normal_mode = true;
  atexit(clear_screen);   //register fun to call after main execution complete , use stack to keep track of which function call first
  atexit(disableRawMode); //disableRawMode() fun get call first since it registered last
  clear_screen();
  
  signal(SIGWINCH, refresh_attributes_and_screen);//refresh screen on window size change 
  
  getcwd(home_dir, 4096);    //get current working directory , it set home_dir with current working directory
  getcwd(current_dir, 4096); //also set current_dir
  int row, col;
  if (getWindowSize(row, col) == -1)
    die("getWindowSize()");

  get_dir_content(home_dir, contents);
  start_index = 0;
  max_line_to_print = min(row / 3, (int)contents.size() - start_index);

  string dir = "/home" +string(current_dir).substr(string(home_dir).size());
  print_directory_content(dir,contents, start_index, max_line_to_print);
  
  cursor_at = {1, 1};
  pair<int,int>dummy;
  draw_footer(row, col,&dummy);
  move_cursor_to(cursor_at.first, cursor_at.second);
}

void change_dir_to(char const *dir)
{
  if (chdir(dir) < 0)
  { //working directory changed
    die("chdir() ");
  }
}
char editorReadKey()
{
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
  {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}
void backspace()
{
  if (strcmp(current_dir,home_dir) == 0)
  { // means it is root dir
    alert(max_line_to_print + 4, "Alert : already at root directory!", cursor_at, RED);
    return;
  }
  string cdir = string(current_dir);

  //put current directory to stack
  left_stack.push(cdir);
  right_stack.clear();

  
  //remove current dir name from current_dir
  int i = cdir.size();
  while (i > 0 && current_dir[i] != '/')
  {
    i--;
  }
  
  current_dir[i] = '\0';

  

  //go to parent directory
  change_dir_to(current_dir);

  //clear screen and repaint
  cursor_at = {1,1};
  refresh_attributes_and_screen(0);
}
void show_upper_content()
{
  if (start_index > 0)
  {
    start_index--;
    move_cursor_to(0, 0);
    string dir = "/home" +string(current_dir).substr(string(home_dir).size());
    print_directory_content(dir, contents, start_index, max_line_to_print);
    if (cursor_at.first < max_line_to_print - 1)
    {
      move_cursor_to(++(cursor_at.first), cursor_at.second);
    }
    else
    {
      move_cursor_to(cursor_at.first, cursor_at.second);
    }
  }
}
void show_lower_content()
{
  if (start_index + max_line_to_print < contents.size())
  {
    //can show lower content
    start_index++;
    move_cursor_to(0, 0);
    string dir = "/home" +string(current_dir).substr(string(home_dir).size());
    print_directory_content(dir,contents, start_index, max_line_to_print);
    if (cursor_at.first > 0)
    {
      move_cursor_to(--(cursor_at.first), cursor_at.second);
    }
    else
    {
      move_cursor_to(cursor_at.first, cursor_at.second);
    }
  }
}
void scroll_up()
{
  if (cursor_at.first > 1)
  {
    move_cursor_to(--(cursor_at.first), cursor_at.second);
  }
  else
  {
    show_upper_content();
    if (cursor_at.first > 1)
    {
      move_cursor_to(--(cursor_at.first), cursor_at.second);
    }
  }
}
void scroll_down()
{
  if (cursor_at.first < max_line_to_print)
  {
    move_cursor_to(++(cursor_at.first), cursor_at.second);
  }
  else
  {
    show_lower_content();
    if (cursor_at.first < max_line_to_print)
    {
      move_cursor_to(++(cursor_at.first), cursor_at.second);
    }
  }
  move_cursor_to(cursor_at.first, cursor_at.second);
}
void enter()
{

  string fileName = contents[cursor_at.first + start_index - 1].first;
  struct stat fileInfo = contents[cursor_at.first + start_index - 1].second;

  if (S_ISDIR(fileInfo.st_mode))
  {
    if (strcmp(fileName.c_str(), "..") == 0)
    {
      //go to parent directory
      backspace();
      return;
    }
    if (strcmp(fileName.c_str(), ".") == 0)
      return;

    //push current dir to stack , clear right_stack
    left_stack.push(current_dir);
    right_stack.clear();

    //traverse current_dir till '\0' and add dir name at end
    int i = 0;
    while (current_dir[i] != '\0')
    {
      i++;
      if (i >= 4096)
        die("enter on dir : "); // in case not found '\0'
    }
    current_dir[i++] = '/';
    string dir_name = fileName;
    int j = 0;
    for (; j < dir_name.size(); j++, i++)
    {
      current_dir[i] = dir_name[j];
    }
    current_dir[i] = '\0';

    change_dir_to(current_dir);
    cursor_at = {1,1};
    refresh_attributes_and_screen(0);
    

    // alert(max_line_to_print + 3 , "parent dir :"+string(current_dir),cursor_at);
  }
  else
  {
    pid_t pid = fork();
    if (pid == 0)
    {
      alert(max_line_to_print + 4, "Alert : File " + fileName + " opened in default editor", cursor_at,GREEN);
      execl("/usr/bin/xdg-open", "xdg-open", fileName.c_str(), NULL);
      exit(1);
    }
  }
  //forwardS.empty();
  return;
}
void left_arrow()
{
  if (left_stack.empty())
  {
    return;
  }
  //push current directory to right_stack
  right_stack.push_back(current_dir);

  //change current_dir
  strcpy(current_dir, left_stack.top().c_str());
  left_stack.pop();

  //go to new directory
  change_dir_to(current_dir);
  cursor_at = {1,1};
  refresh_attributes_and_screen(0);
}
void right_arrow()
{
  if (right_stack.empty())
  {
    return;
  }
  //push current_dir to left_stack
  left_stack.push(current_dir);

  //update current_dir with top of right_stack
  strcpy(current_dir, right_stack.back().c_str());
  right_stack.pop_back();

  //go to new directory
  change_dir_to(current_dir);
  cursor_at = {1,1};
  refresh_attributes_and_screen(0);
}
void print_space(int n)
{
  for (int i = 0; i < n; i++)
  {
    cout << " ";
  }
}
void go_home()
{
  if (strcmp(current_dir,home_dir) == 0)
  { // means it is root dir
    alert(max_line_to_print + 4, "Alert : already at home directory!", cursor_at, RED);
    return;
  }
  //push current_dir to left_stack and clear right_stack
  left_stack.push(current_dir);
  right_stack.clear();

  //set current_dir with home_dir
  strcpy(current_dir, home_dir);
  change_dir_to(current_dir);
  cursor_at = {1,1};
  refresh_attributes_and_screen(0);
}
bool valid_path(string &path)
{
  switch (path[0])
  {
  case '~':
    path.replace(0, 1, home_dir);
    break;
  case '.':
    path.replace(0, 1, current_dir);
    break;

  case '/':
    //this means path is absolute from root
    break;
  default:
    return false;
  }
  return true;
}

vector<string> read_command(pair<int, int> cursor)
{
  char c;
  vector<string> res;
  pair<int, int> local_cursor = cursor;
  while (c != ESC) //till esc not press run this while loop
  {
    cin.get(c);
    move_cursor_to(local_cursor.first + 2, 1);
    clearLine();
    move_cursor_to(local_cursor.first, local_cursor.second); //it is to remove previouse error message
    if (c == ENT)                                            //on enter press erase command and return it
    {
      if (typed_command_till_now == "")
      {
        res.push_back("");
        return res;
      }
      move_cursor_to(cursor.first, cursor.second);
      print_space(typed_command_till_now.size() + 2);
      move_cursor_to(cursor.first, cursor.second);

      //string tokenization
      string token;
      for( int i=0; i<typed_command_till_now.size(); i++){
          char c = typed_command_till_now[i];
          if( c == ' ' ){
              if(token != "")
              res.push_back(token);
              token = "";
          }else if(c == '\\' ){
              token+=' ';
              i++;
          }else{
              token += c;
          }
      }
      if(token != "")
      res.push_back(token);

      typed_command_till_now.clear();
      return res;
    }
    else if (c == BKSP)
    {
      if (typed_command_till_now.size() > 0)
      {
        move_cursor_to(local_cursor.first, --(local_cursor.second));
        print_space(3);
        move_cursor_to(local_cursor.first, local_cursor.second);
        typed_command_till_now.pop_back();
      }
      continue;
    }
    cout << c;
    local_cursor.second++;
    typed_command_till_now += c;
  }
  res.push_back("esc");
  typed_command_till_now.clear();
  return res;
}
void switch_to_command_mode()
{
  normal_mode = false;
  int row, col;
  if (getWindowSize(row, col) == -1)
    die("getWindowSize() : ");
  pair<int, int> local_cursor;
  draw_footer(row, col, &local_cursor); //it set local_cursor to cursor position
  vector<string> command_line;
  string command;
  while (1)
  {
    command_line = read_command(local_cursor);
    command = command_line[0];
    //remove command from command_line
    command_line.erase(command_line.begin(), command_line.begin() + 1);
    if (command == "esc")
    { //on esc press, go to normal mode
      normal_mode = true;
      break;
    }
    else if (command == "copy")
    {
      string destination_path = command_line.back();
      //removing destination path so that command_line contain only file names
      command_line.pop_back();
      //update all source file paths
      bool check = false;
      for(auto file:command_line){
        if(!valid_path(file)){
          alert(local_cursor.first + 2, "Invalid source path(s) !", local_cursor,RED);
          check = true;
          break;
        }
      }
      if(check)//there is some invalid path
        continue;
      if (valid_path(destination_path))
      {
        copy_command(command_line, destination_path );
        alert(local_cursor.first + 2, "Copy successfull!", local_cursor,GREEN);
      }
      else
      {
        alert(local_cursor.first + 2, "Invalid destination path !", local_cursor,RED);
        continue;
      }
    }
    else if (command == "move")
    {
      //update source file path and destination file path
      bool check = false;
      for(auto file:command_line){
        if(!valid_path(file)){
          alert(local_cursor.first + 2, "Invalid path(s) !", local_cursor,RED);
          check = true;
          break;
        }
      }
      if(check)//there is some invalid path
        continue;
      
      string destination_path = command_line.back();
      //removing destination path so that command_line contain only file names
      command_line.pop_back();

      move_command(command_line,destination_path);
      refresh_attributes_and_screen(0);
      alert(local_cursor.first + 2, "Move successfull !", local_cursor,GREEN);
    }
    else if (command == "rename")
    {
      if (command_line.size() != 2)
      {
        alert(local_cursor.first + 2, "Invalid names provided !", local_cursor,RED);
        continue;
      }
      //update old file path and new file path
      bool check = false;
      for(auto file:command_line){
        if(!valid_path(file)){
          alert(local_cursor.first + 2, "Invalid path(s) !", local_cursor,RED);
          check = true;
          break;
        }
      }
      if(check)//there is some invalid path
        continue;
      if (rename(command_line[0].c_str(), command_line[1].c_str()) != 0)
      {
        alert(local_cursor.first + 2, "File not exist !", local_cursor,RED);
        continue;
      }
      refresh_attributes_and_screen(0);
      alert(local_cursor.first + 2, "Rename successfull !", local_cursor,GREEN);
    }
    else if (command == "create_file")
    {
      if(command_line.size() < 2){
        alert(local_cursor.first + 2, "Invalid command !", local_cursor,RED);
        continue;
      }
      string path = command_line.back();
      //removing file path so that command_line contain only file names
      command_line.pop_back();
      if (valid_path(path))
      {
        create_file_command(path, command_line);
        refresh_attributes_and_screen(0);
        alert(local_cursor.first + 2, "File created successfull!", local_cursor,GREEN);
      }
      else
      {
        alert(local_cursor.first + 2, "Invalid Path !", local_cursor,RED);
        continue;
      }
    }
    else if (command == "create_dir")
    {
      if(command_line.size() < 2){
        alert(local_cursor.first + 2, "Invalid command !", local_cursor,RED);
        continue;
      }
      string path = command_line.back();
      //removing file path so that command_line contain only file names
      command_line.pop_back();
      if (valid_path(path))
      {
        create_dir_command(path, command_line);
        refresh_attributes_and_screen(0);
        alert(local_cursor.first + 2, "Directory created successfull!", local_cursor,GREEN);
      }
      else
      {
        alert(local_cursor.first + 2, "Invalid Path !", local_cursor,RED);
      }
    }
    else if (command == "delete_file")
    {
      string path = command_line.back();
      //removing file path so that command_line contain only file names
      command_line.pop_back();
      if (valid_path(path))
      {
        delete_file_command(path);
        refresh_attributes_and_screen(0);
        alert(local_cursor.first + 2, "delete successfull!->"+path, local_cursor,GREEN);
      }
      else
      {
        alert(local_cursor.first + 2, "Invalid Path !", local_cursor,RED);
      }
    }
    else if (command == "delete_dir")
    {
      string path = command_line.back();
      if (valid_path(path))
      {
        char temp[path.size()+2];
        strcpy(temp,path.c_str());
        delete_dir_command(temp,path.size()+2);
        refresh_attributes_and_screen(0);
        alert(local_cursor.first + 2, "Directory deleted successfully!", local_cursor,GREEN);
      }
      else
      {
        alert(local_cursor.first + 2, "Invalid Path !", local_cursor,RED);
      }
    }
    else if (command == "goto")
    {
      string path = command_line.back();
      if (valid_path(path))
      {
        //push current_dir to left_stack and clear right_stack
        left_stack.push(current_dir);
        right_stack.clear();

        //set current_dir with path
        strcpy(current_dir, path.c_str());
        change_dir_to(current_dir);
        refresh_attributes_and_screen(false);
        alert(local_cursor.first + 2, "your are at Destination Directory", local_cursor,GREEN);
      }
      else
      {
        alert(local_cursor.first + 2, "Invalid Path !", local_cursor,RED);
      }
    }
    else if (command == "search"){
      string file_or_folder = command_line.back();
      if (search_command(file_or_folder ,current_dir, contents))
      {
        alert(local_cursor.first + 2, "Output : true", local_cursor,GREEN);
      }
      else
      {
        alert(local_cursor.first + 2, "Output : false", local_cursor,RED);
      }
    }
    else
    {
      alert(local_cursor.first + 2, "-Wrong command", local_cursor,RED);
    }
  }
  pair<int,int>dummy;
  draw_footer(row, col,&dummy);
  move_cursor_to(cursor_at.first, cursor_at.second);
}
void editorProcessKeypress()
{

  char c = editorReadKey();
  switch (c)
  {
  case 'q':
    exit(0);
    break;
  case ':':
    switch_to_command_mode();
    break;

  case ENT:
    enter();
    break;

  case BKSP:
    backspace();
    break;

  case UP:
    scroll_up();
    break;

  case LEFT:
    left_arrow();
    break;

  case K:
    show_upper_content();
    break;

  case H:
    go_home();
    break;

  case DOWN:
    scroll_down();
    break;

  case RIGHT:
    right_arrow();
    break;

  case L:
    show_lower_content();
    break;

  default:
    break;
  }
}
int main(int argc, char const *argv[])
{
  char c;
  init();
  while (true)
  {
    //refresh_attributes_and_screen(false);
    editorProcessKeypress();
  }
  //processCurrentDIR((string(cwd)+'/'+string(fileName)).c_str());
  return 0;
}
