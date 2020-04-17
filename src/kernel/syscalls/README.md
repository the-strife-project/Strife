# jotaOS system calls

The system calls are done by setting the eax register to an index, some other registers as parameters, and performing an `int 0x69`.

| eax | System call | edi | esi | edx | ecx |
| --- | --- | --- | --- | --- | --- |
| 1 | sys_write | uint32_t fd | char* buf | uint32_t count |
