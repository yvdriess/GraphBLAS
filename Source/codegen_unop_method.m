function codegen_unop_method (unop, op, fcast, ztype, xtype)
%CODEGEN_UNOP_METHOD create a function to compute C=unop(cast(A))
%
% codegen_unop_method (unop, op, fcast, ztype, xtype)
%
%   unop: the name of the operator
%   op: a string defining the computation
%   ztype: the type of z for z=f(x)
%   xtype: the type of x for z=f(x)

f = fopen ('control.m4', 'w') ;

[zname, zunsigned, zbits] = codegen_type (ztype) ;
[xname, xunsigned, xbits] = codegen_type (xtype) ;

name = sprintf ('%s_%s_%s', unop, zname, xname) ;

is_identity = isequal (unop, 'identity') ;
no_typecast = isequal (ztype, xtype) ;

% function names
if (is_identity && no_typecast)
    % disable this worker
    fprintf (f, 'define(`GB_unop'', `(none)'')\n', name) ;
    fprintf (f, 'define(`if_operator_is_enabled'', `#if 0'')\n') ;
    fprintf (f, 'define(`endif_operator_is_enabled'', `#endif'')\n') ;
else
    fprintf (f, 'define(`GB_unop'', `GB_unop__%s'')\n', name) ;
    fprintf (f, 'define(`if_operator_is_enabled'', `'')\n') ;
    fprintf (f, 'define(`endif_operator_is_enabled'', `'')\n') ;
end
fprintf (f, 'define(`GB_tran'', `GB_tran__%s'')\n', name) ;

% type of C and A
fprintf (f, 'define(`GB_ctype'', `%s'')\n', ztype) ;
fprintf (f, 'define(`GB_atype'', `%s'')\n', xtype) ;

A_is_pattern = (isempty (strfind (op, 'xarg'))) ;

% to get an entry from A
if (A_is_pattern)
    % A(i,j) is not needed
    fprintf (f, 'define(`GB_geta'', `;'')\n') ;
else
    fprintf (f, 'define(`GB_geta'', `%s $1 = $2 [$3]'')\n', xtype) ;
end

% type-specific IMINV
if (~isempty (strfind (op, 'IMINV')))
    if (zunsigned)
        op = strrep (op, 'IMINV', 'IMINV_UNSIGNED') ;
    else
        op = strrep (op, 'IMINV', 'IMINV_SIGNED') ;
    end
    op = strrep (op, ')', sprintf (', %d)', zbits)) ;
end

% create the unary operator
op = strrep (op, 'xarg', '`$2''') ;
fprintf (f, 'define(`GB_UNARYOP'', `$1 = %s'')\n', op) ;

% create the cast operator
if (A_is_pattern)
    % cast (A(i,j)) is not needed
    fprintf (f, 'define(`GB_cast'', `;'')\n') ;
else
    fcast = strrep (fcast, 'zarg', '`$1''') ;
    fcast = strrep (fcast, 'xarg', '`$2''') ;
    fprintf (f, 'define(`GB_cast'', `%s'')\n', fcast) ;
end

% create the disable flag
disable  = sprintf ('GxB_NO_%s', upper (unop)) ;
disable = [disable (sprintf (' || GxB_NO_%s', upper (zname)))] ;
if (~isequal (zname, xname))
    disable = [disable (sprintf (' || GxB_NO_%s', upper (xname)))] ;
end
fprintf (f, 'define(`GB_disable'', `(%s)'')\n', disable) ;
fclose (f) ;

% ff = fopen ('temp.h', 'a') ;
% fprintf (ff, '// #define GxB_NO_%s\n', upper (unop)) ;
% fclose (ff) ;

% construct the *.c file
cmd = sprintf (...
'cat control.m4 Generator/GB_unaryop.c | m4 | tail -n +11 > Generated/GB_unaryop__%s.c', ...
name) ;
fprintf ('.') ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_unaryop.h | m4 | tail -n +11 >> Generated/GB_unaryop__include.h') ;
system (cmd) ;

delete ('control.m4') ;

