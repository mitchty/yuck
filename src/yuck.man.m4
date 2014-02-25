./* auto generated by yuck -*- nroff -*-
changequote`'changequote([,])dnl
ifdef([YUCK_VERSION], [], [dnl else (!YUCK_VERSION)
ifdef([YUCK_SCMVER_VERSION], [dnl then
define([YUCK_VERSION], [YUCK_SCMVER_VERSION])[]dnl
], [dnl else (!YUCK_SCMVER_VERSION)
define([YUCK_VERSION], [unknown])dnl
])dnl
])dnl
define([ucase], [translit([$1], [abcdefghijklmnopqrstuvwxyz], [ABCDEFGHIJKLMNOPQRSTUVWXYZ])])dnl
ifdef([YUCK_PKG_STR], [], [define([YUCK_PKG_STR], [YUCK_UMB_STR])])dnl
.TH ucase(YUCK_UMB_STR) "1" "YUCK_MAN_DATE" "YUCK_PKG_STR YUCK_VERSION" "User Commands"
.SH NAME
pushdef([umb_desc], [yuck_first_line(yuck_umb_desc())])dnl
pushdef([alt_desc], [manual page for YUCK_PKG_STR YUCK_VERSION])dnl
YUCK_UMB_STR - first_nonnil(umb_desc, alt_desc)
popdef([umb_desc])dnl
popdef([alt_desc])dnl
.SH SYNOPSIS
.B YUCK_UMB_STR
[[\fIOPTION\fR]]...[ ]dnl
ifelse(yuck_cmds(), [], [], [\fICOMMAND\fR])dnl
[]dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(translit(defn([YUCK_UMB_POSARG]), [[]], []),
[ -], [\fR -]),
[ ], [ \fI]),
[], [\fR\fI]),
[], [\fR]),
[...], [\fR...])
.SH DESCRIPTION
yuck_umb_desc()
ifelse(yuck_cmds(), [], [], [dnl

.PP
\fICOMMAND\fR may be one of:
foreachq([__CMD__], yuck_cmds(), [dnl
.TP
.B yuck_cmd_string(__CMD__)
.
yuck_cmd_desc(__CMD__)
])dnl
])dnl

.PP
define([yuck_man_option], [dnl
pushdef([lhs], [yuck_option_help_lhs([$1], [$2])])dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(lhs,
[,], [\fR,]),
[ -], [ \fB-]),
[], [\fR]),
[=], [\fR=]),
[=], [=\fI]),
[], [\fR]),
[...], [\fR...])dnl
popdef([lhs])dnl
])
Recognized \fIOPTION\fRs:
foreachq([__IDN__], yuck_idents(), [dnl
.TP
.B yuck_man_option(defn([__IDN__]), [])
yuck_option_desc(defn([__IDN__]), [])
])dnl
dnl
ifelse(yuck_cmds(), [], [], [dnl
.SH COMMANDS
])dnl
foreachq([__CMD__], yuck_cmds(), [
.P
.B YUCK_UMB_STR yuck_cmd_string(__CMD__)
[[\fIOPTION\fR]]...
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(dnl
yuck_esc(translit(yuck_cmd_posarg(__CMD__), [[]], []),
[ -], [\fR -]),
[ ], [ \fI]),
[], [\fR\fI]),
[], [\fR]),
[...], [\fR...])
.br
yuck_cmd_desc(C)
.P
\fIOPTION\fRs specific to the \fB[]yuck_cmd_string(__CMD__)\fR command:
foreachq([__IDN__], yuck_idents(__CMD__), [dnl
.TP
.B yuck_option_help_lhs(defn([__IDN__]), defn([__CMD__]))
yuck_option_desc(defn([__IDN__]), defn([__CMD__]))
])dnl
])dnl
foreachq([__INC__], defn([YUCK_INCLUDES]), [dnl
include(__INC__)
])
./* yuck.m4man ends here
changequote`'dnl
