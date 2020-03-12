//
// This file is part of the SPNC project.
// Copyright (c) 2020 Embedded Systems and Applications Group, TU Darmstadt. All rights reserved.
//

#ifndef SPNC_JOB_H
#define SPNC_JOB_H

#include <vector>
#include <memory>
#include <type_traits>
#include "Actions.h"

namespace spnc {

  /// Jobs are generated by toolchains and contain a sequence of actions. When executed,
  /// the Job will run all contained actions in the order specified by the dependencies between the
  /// actions to produce the output.
  /// The Job also acts as a container for the actions and controls there lifetime.
  /// \tparam Output Type of the output produced by this job.
  template<typename Output>
  class Job {

  public:

    /// Create and insert an action into this Job. The Job controls the lifetime of the created action.
    /// \tparam A Type of the action to create.
    /// \tparam T Types of the arguments required for the action's constructor.
    /// \param args Arguments to the action's constructor.
    /// \return A reference to the created action.
    template<typename A, typename ...T>
    A& insertAction(T&& ... args) {
      static_assert(std::is_base_of<ActionBase, A>::value, "Must be an action derived from ActionBase!");
      actions.push_back(std::make_unique<A>(std::forward<T>(args)...));
      return *((A*) actions.back().get());
    }

    /// Create and insert the final into this Job. The Job controls the lifetime of the created action.
    /// The created action's output type must match that of the Job.
    /// \tparam A Type of the action to create.
    /// \tparam T Types of the arguments required for the action's constructor.
    /// \param args Arguments to the action's constructor.
    /// \return A reference to the created, final action.
    template<typename A, typename ...T>
    A& insertFinalAction(T&& ... args) {
      static_assert(std::is_base_of<ActionWithOutput<Output>, A>::value, "Must be an action with correct output!");
      auto a = std::make_unique<A>(std::forward<T>(args)...);
      finalAction = a.get();
      actions.push_back(std::move(a));
      return *((A*) actions.back().get());
    }

    /// Insert a previously created action. Assumes ownership of the action.
    /// In general, the use of insertAction should be preferred over this method.
    /// \param action The action to insert.
    /// \return A reference to the inserted action.
    ActionBase& addAction(std::unique_ptr<ActionBase> action) {
      actions.push_back(std::move(action));
      return *actions.back();
    }

    /// Insert the previously created final action. Assumes ownership of the action.
    /// The output type of the action must match the output type of the Job.
    /// In general, the use of insertFinalAction should be preferred over this method.
    /// \return A reference to the final action.
    ActionBase& setFinalAction(std::unique_ptr<ActionWithOutput < Output>>
    action) {
      finalAction = action.get();
      actions.push_back(std::move(action));
      return *actions.back();
    }

    /// Execute the job, i.e., all contained actions according to their order.
    /// Eventually yields the output of the job.
    /// \return The output of the action as generated by the final action.
    Output& execute() {
      return finalAction->execute();
    }

  private:

    std::vector<std::unique_ptr<ActionBase>> actions;

    ActionWithOutput <Output>* finalAction;

  };
}

#endif //SPNC_JOB_H
