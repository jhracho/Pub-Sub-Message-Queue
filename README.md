[![Work in Repl.it](https://classroom.github.com/assets/work-in-replit-14baed9a392b3a25080506f3b7b6d57f295ec2978f6f33ec97e36a161684cbe9.svg)](https://classroom.github.com/online_ide?assignment_repo_id=281633&assignment_repo_type=GroupAssignmentRepo)
# Project 02: Message Queue

This is [Project 02] of [CSE.30341.FA20].

## Students

1. Jake Hracho (jhracho@nd.edu)
2. Chris Hunt (chunt4@nd.edu)

## Brainstorming

The following are questions that should help you in thinking about how to
approach implementing [Project 02].  For this project, responses to these
brainstorming questions **are not required**.

### Request

1. What data must be allocated and deallocated for each `Request` structure?

2. What does a valid **HTTP** request look like?

### Queue

1. What data must be allocated and deallocated for each `Queue` structure?

2. How will you implement **mutual exclusion**?

3. How will you implement **signaling**?

3. What are the **critical sections**?

### Client

1. What data must be allocated and deallocated for each `MessageQueue`
   structure?

2. What should happen when the user **publishes** a message?

3. What should happen when the user **retrieves** a message?

4. What should happen when the user **subscribes** to a topic?

5. What should happen when the user **unsubscribes** to a topic?

6. What needs to happen when the user **starts** the `MessageQueue`?

7. What needs to happen when the user **stops** the `MessageQueue`?

8. How many internal **threads** are required?

9. What is the purpose of each internal **thread**?

10. What `MessageQueue` attribute needs to be **protected** from **concurrent**
    access?

## Demonstration

> Link to **video demonstration** of **chat application**.
https://youtu.be/hTC9nnQAFYU

## Errata

> Describe any known errors, bugs, or deviations from the requirements.

[Project 02]:       https://www3.nd.edu/~pbui/teaching/cse.30341.fa20/project02.html
[CSE.30341.FA20]:   https://www3.nd.edu/~pbui/teaching/cse.30341.fa20/
