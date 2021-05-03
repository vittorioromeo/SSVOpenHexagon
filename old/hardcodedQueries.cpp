
        if(splitted[1] == "add")
        {
            if(splitted.size() < 3)
            {
                SSVOH_SLOG_ERROR << "'db add' command must be followed by a "
                                    "valid database object type\n";

                return true;
            }

            if(splitted[2] == "user")
            {
                if(splitted.size() < 4)
                {
                    SSVOH_SLOG_ERROR
                        << "'db add user' command must be followed by a "
                           "pipe-separated list of values\n";

                    return true;
                }

                const auto userData =
                    Utils::split<std::string>(splitted[3], "|");

                if(userData.size() < 2)
                {
                    SSVOH_SLOG_ERROR << "'db add user' requires 2 values\n";
                    return true;
                }

                const Database::User toAdd{
                    .id = static_cast<std::uint32_t>(-1),
                    .steamId = std::stoull(userData[0]),
                    .name = std::string{userData[1]} //
                };

                Database::addUser(toAdd);
                return true;
            }

            SSVOH_SLOG_ERROR << "Unknown object type '" << splitted[2]
                             << "' for 'db add' command\n";

            return true;
        }

        if(splitted[1] == "remove")
        {
            if(splitted.size() < 3)
            {
                SSVOH_SLOG_ERROR << "'db remove' command must be followed by a "
                                    "valid database object type\n";

                return true;
            }

            if(splitted[2] == "user")
            {
                if(splitted.size() < 4)
                {
                    SSVOH_SLOG_ERROR << "'db remove user' command must be "
                                        "followed by a valid id\n";

                    return true;
                }

                const std::uint32_t toRemove = std::stoul(splitted[3]);

                Database::removeUser(toRemove);
                return true;
            }

            SSVOH_SLOG_ERROR << "Unknown object type '" << splitted[2]
                             << "' for 'db add' command\n";

            return true;
        }

        if(splitted[1] == "dump")
        {
            if(splitted.size() < 3)
            {
                SSVOH_SLOG_ERROR << "'db dump' command must be followed by a "
                                    "valid database table name\n";

                return true;
            }

            if(splitted[2] == "users")
            {
                Database::dumpUsers();
                return true;
            }

            SSVOH_SLOG_ERROR << "Unknown database table name '" << splitted[2]
                             << "' for 'db dump' command\n";

            return true;
        }
