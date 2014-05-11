function p_call(varargin)
        vals = get(h.c,'Value');
        checked = find([vals{:}])
        if isempty(checked)
            checked = 'none';
        end
        save('Exercises.mat','Choices','checked');
        disp(checked)
    end