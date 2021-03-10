//
// Created by fgrotsch on 3/10/21.
//

#ifndef DAQMODULETEST_ISSUES_H
#define DAQMODULETEST_ISSUES_H

#include "appfwk/DAQModule.hpp"
#include "ers/Issue.hpp"

#include <string>

namespace dunedaq {
    ERS_DECLARE_ISSUE_BASE(daqmoduletest,
            QueueFatalError,
            appfwk::GeneralDAQModuleIssue,
    "The " << queueType << " queue was not successfully created.",
    ((std::string)name),
    ((std::string)queueType))

    ERS_DECLARE_ISSUE_BASE(daqmoduletest,
                           FileError,
                           appfwk::GeneralDAQModuleIssue,
                           "The file " << file_name << "could not be opened.",
                           ((std::string) name),
                            ((std::string)file_name))

}

#endif //DAQMODULETEST_ISSUES_H
