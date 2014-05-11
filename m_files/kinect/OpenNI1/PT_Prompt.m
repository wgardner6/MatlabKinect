%Prompts the user to enter username and password and passes it back to
%PT_Choices
function user = PT_Prompt
prompt = {'Enter username:','Enter password:'};
dlg_title = 'LogIn';
num_lines = 1;
user = inputdlg(prompt,dlg_title,num_lines);
end