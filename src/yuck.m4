changequote([,])dnl
divert([-1])

## this is a little domain language for the yuck processor

# foreachq(x, [item_1, item_2, ..., item_n], stmt)
#   quoted list, alternate improved version
define([foreachq], [ifelse([$2], [], [],
	[pushdef([$1])_$0([$1], [$3], [], $2)popdef([$1])])])
define([_foreachq], [ifelse([$#], [3], [],
	[define([$1], [$4])$2[]$0([$1], [$2],
		shift(shift(shift($@))))])])

define([append], [dnl
	define([$1], ifdef([$1], [defn([$1])[$3]])[$2])
])
define([append_ne], [dnl
	## like append, but don't append empty arguments
	ifelse([$2], [], [], [append([$1], [$2], [$3])])
])

define([yuck_set_umbrella], [dnl
	define([YUCK_UMB], [$1])
])
define([yuck_add_command], [dnl
	pushdef([cmd], [$1])
	pushdef([str], [ifelse([$#], [2], [$2], [$1])])
	append([YUCK_CMD], cmd, [,])
	define([YUCK_STR.]cmd, str)
	popdef([cmd])
	popdef([str])
])
define([yuck_add_option], [dnl
	## yuck_add_option([CMD], short, long, type)
	pushdef([cmd], [$1])
	pushdef([short], [$2])
	pushdef([long], [$3])
	pushdef([type], [$4])

	pushdef([slotname], ifelse(long, [], ifelse(short, [], [define([cnt], ifdef([cnt], [incr(cnt)], [0]))[s]cnt], [dash]short), long)[_]type)
	pushdef([slot], ifelse(type, [], [void slotname],
		type, [flag], [unsigned int slotname],
		type, [arg], [const char *slotname],
		type, [marg], [const char **slotname]))

	append_ne([YUCK.]cmd[.S], short, [,])
	append_ne([YUCK.]cmd[.L], long, [,])
	append_ne([YUCK.]cmd[.SLOT], slot, [,])

	define([YUCK.]cmd[.]short[.slot], ifelse(cmd, [], slotname, cmd.slotname))
	define([YUCK.]cmd[.]long[.slot], ifelse(cmd, [], slotname, cmd.slotname))
	define([YUCK.]cmd[.]short[.type], type)
	define([YUCK.]cmd[.]long[.type], type)

	popdef([slot])
	popdef([cmd])
	popdef([short])
	popdef([long])
	popdef([type])
])


## test case
yuck_set_umbrella([yuck])
yuck_add_option([], [], [help], [flag])
yuck_add_command([gen])
yuck_add_option([gen], [x], [extra], [arg])
yuck_add_option([gen], [h], [], [flag])
yuck_add_option([gen], [], [version], [flag])
yuck_add_command([tst])
yuck_add_option([tst], [], [version], [flag])
yuck_add_option([tst], [], [], [flag])
yuck_add_option([tst], [], [], [flag])

dnl define([YUCK_SET], [dnl
dnl pushdef([slot], [YUCK_SLOT_MAP([slot], [$2])])dnl
dnl pushdef([flav], [YUCK_SLOT_MAP([flav], [$2])])dnl
dnl dnl
dnl ifelse(slot, [], [/* no [slot] defined for `$2' */],
dnl 	flav, [flag], [$1->slot[_given++]],
dnl 	flav, [arg], [$1->slot[_arg = ]$3],
dnl 	flav, [marg], [],
dnl 	flav, [], [/* no [flavour] defined for `$2' */])dnl
dnl popdef([slot])dnl
dnl popdef([flav])dnl
dnl ])

define([yield], [goto $1; back_from_$1:])
define([coroutine], [define([this_coru], [$1])$1:])
define([resume], [goto back_from_[]this_coru])
define([quit], [goto out])

divert[]dnl