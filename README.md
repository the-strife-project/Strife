This repo is the official jotaOS distribution.

Since all projects have their own repository in [this GitHub organization](https://github.com/jotaOS), there has to be a way to glue them all together.

This distribution is stable: it will never intentionally contain unfinished parts that break everything. It also has a simple Makefile which compiles all the programs in [the `projects` directory](https://github.com/jotaOS/jotaOS/tree/master/projects) and creates an ISO file. In the very near future, it will also have a CI set up so you can just download the latest ISO.