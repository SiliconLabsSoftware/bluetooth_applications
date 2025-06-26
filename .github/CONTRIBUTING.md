# Contributing Guideline
As an open-source project, we welcome and encourage the community to submit patches directly to the project.  
In our collaborative open-source environment, standards and methods for submitting changes help reduce  
the chaos that can result from an active development community.

This document explains how to participate in project conversations, log bugs and enhancement requests,  
and submit patches to the project so your patch will be accepted quickly into the codebase.

## Prerequisites
You should be familiar with Git and GitHub. [Getting started](https://docs.github.com/en/get-started)  
If you haven't already done so, you'll need to create a (free) GitHub account at https://github.com  
and have Git tools available on your development system. You also need to add your email address to your account.

As a contributor, you'll want to be familiar with the Silicon Labs tooling:
- [Simplicity Studio](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-overview/)  
- [Platform](https://docs.silabs.com/gecko-platform/latest/platform-overview/)  
- [Simplicity Commander](https://docs.silabs.com/simplicity-commander/latest/simplicity-commander-start/)  

Read the Silicon Labs [coding guidelines](https://github.com/SiliconLabsSoftware/agreements-and-guidelines/blob/main/coding_standard.md).  
## Git Setup
We need to know who you are, and how to contact you. Please ass the following information to your Git installation:
```
git config --global user.name "FirstName LastName"
git config --global user.email "firstname.lastname@example.com"
```
set the Git configuration variables user.name to your full name, and user.email to your email address.
The user.name must be your full name (first and last at minimum), not a pseudonym or hacker handle.  
The email address that you use in your Git configuration must match the email address you use to sign your commits.  

If you intend to edit commits using the Github.com UI, ensure that your github profile email address and profile name also match those used in your git configuration  
(user.name & user.email).

### Set up GitHub commit signature

**command line setup**

The repository requires signed off commits. Follow this [guide](https://docs.github.com/en/authentication/managing-commit-signature-verification/signing-commits) how to set it up.
1. Generate a gpg key [howto](https://docs.github.com/en/authentication/managing-commit-signature-verification/generating-a-new-gpg-key)
2. Configure your local repository with the gpg key. [guide]whttps://docs.github.com/en/authentication/managing-commit-signature-verification/telling-git-about-your-signing-key)
3. Configure your GitHub account with the gpg key [guide](https://docs.github.com/en/authentication/managing-commit-signature-verification/associating-an-email-with-your-gpg-key)

**Command line steps:**  
Use the git-bash and navigate into your local repo.
1. disable all the gpg signature globally. (Optional)
```
$ git config --global --unset gpg.format
```
2. Create a gpg-key
```
$ gpg --full-generate-key
```
3. Configure the local repo with your new key.
```
$ gpg --list-secret-keys --keyid-format=long
gpg: checking the trustdb
gpg: marginals needed: 3  completes needed: 1  trust model: pgp
gpg: depth: 0  valid:   1  signed:   0  trust: 0-, 0q, 0n, 0m, 0f, 1u
/c/Users/silabsuser/.gnupg/pubring.kbx
------------------------------------
sec   rsa3072/1234567891234567 2025-04-09 [SC]
      ABDGDGFDGFDGDHHSRGRG12345667912345678981
uid                 [ultimate] Firstname Lastname <example@example.com>
ssb   rsa3072/11098765432110981 2025-04-09 [E]

$ git config user.signingkey 1234567891234567
```
4. Force every commit to be signed
```
$ git config commit.gpgsign true
```
5. Export your gpg key
```
$ gpg --armor --export 888BA795B7085898
```
Make sure your email address is verified by GitHub before committing anything.

## Licensing
Please check the [Licensing.md](../LICENSE.md) for more details.

## Contributor License Agreement
When a project receives a contribution, it must be clear that the contributor has the rights to contribute the content and that the project then has the rights to use and otherwise operate with the content (e.g., relicense or distribute). A Contributor License Agreement (CLA) is a legal document establishing these rights and defining the terms under which a license is granted by a contributor to an open-source project. A CLA clarifies that any contribution was authorized (not contributing someone else’s code without permission or without legal authority to contribute) and protects the project from potential future legal challenges.

Please check Silicon Labs [CLA document](https://github.com/SiliconLabsSoftware/agreements-and-guidelines/blob/main/contributor_license_agreement.md).  
During the pull request review, every new contributor must sign the CLA document. It can be signed as an individual or on behalf of a company.  
Signatures have a 6-month expiration period.

## Contribution process
### Creating an Issue
Please follow the official GitHub [guide](https://opensource.guide/how-to-contribute/#opening-an-issue).

### Fork the repository
When you created an issue and based on the discussion you want to contribute with your source-code.
Branching is disabled on the public Silicon Labs repositories. You need to fork your own repo first.
Please follow the official GitHub [guide](https://docs.github.com/en/get-started/exploring-projects-on-github/contributing-to-a-project).
You can create your branch on your own forked repo now.

### Branch Naming Convention
Branch naming shall follow the following template: *IssueNumber-issue-title-goes-here*  
Example branch name:
```
99-bootloader-implementation
```
Issue number is necessary to maintain tracebility.
Now you have a branch. You can start committing your code onto it.

## Commit Messages

Silicon Labs repositories require signed-off commits.
Every commit represents a change inside the repository. Every change needs to be documented extensively.  
```
Issuenumber-summary-of-changes

Detailed description what was implemented.
Another line of really good description
```

## Pull Request Guideline
Okay you finished your work committed all your changes to your branch. Time to create a pull request.  
Refer to the general pull request [guideline](https://opensource.guide/how-to-contribute/#opening-a-pull-request) from GitHub.  
What to consider when raising a Pull Request:  
1. **Pull Request Naming**  
   By default, GitHub uses the branch name as the pull request title. If the branch naming convention was followed, no changes are needed here.
2. **Create Description**
   Fill out the pull request template.
3. **Check the Reviewer List**  
   GitHub assigns reviewers based on the [CODEOWNERS](CODEOWNERS) file.  
   Add more reviewers if needed. Do not remove reviewers from the PR. Ask the repository owner for updates to the code owners.

### As a Reviewer

What to consider when reviewing a Pull Request:

- All builds must pass successfully.
- The code must follow the Silicon Labs [coding guidelines](https://github.com/SiliconLabsSoftware/agreements-and-guidelines/blob/main/coding_standard.md).
- Write clear comments. Describe the issue and explain why you disagree (e.g., mistakes, errors, violations of conventions, performance risks, security issues, etc.).
- If any comments must be addressed mandatorily, mark the pull request as “Draft.”