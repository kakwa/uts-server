pipeline {
  agent none
  stages {
    stage('Compile') {
      steps {
        parallel(
          "OpenBSD 6.1": {
            node(label: 'openbsd-6.1') {
              sh 'git config --global user.name "jenkins@kakwa.fr"'
              sh 'git config --global user.email "jenkins@kakwa.fr"'
              git 'https://github.com/kakwa/uts-server'
              sh 'git clean -fdx'
              sh 'export CC=/usr/local/bin/egcc;export CXX=/usr/local/bin/ec++; cmake . -DBUNDLE_CIVETWEB=ON'
              sh 'export CC=/usr/local/bin/egcc;export CXX=/usr/local/bin/ec++; make -j4'
            }
            
            
          },
          "FreeBSD 11": {
            node(label: 'freebsd-11') {
              sh 'git config --global user.email "jenkins@kakwa.fr"'
              sh 'git config --global user.name "jenkins@kakwa.fr"'
              git 'https://github.com/kakwa/uts-server'
              sh 'git clean -fdx'
              sh 'cmake . -DBUNDLE_CIVETWEB=ON'
              sh 'make -j4'
            }
            
            
          },
          "CentOS 7": {
            node(label: 'centos-7') {
              sh 'git config --global user.email "jenkins@kakwa.fr"'
              sh 'git config --global user.name "jenkins@kakwa.fr"'
              git 'https://github.com/kakwa/uts-server'
              sh 'git clean -fdx'
              sh 'export CXX=/usr/bin/clang++; export CC=/usr/bin/clang; cmake . -DBUNDLE_CIVETWEB=ON'
              sh 'export CXX=/usr/bin/clang++; export CC=/usr/bin/clang; make -j4'
            }
            
            
          },
          "Debian 8": {
            node(label: 'debian-8') {
              sh 'git config --global user.email "jenkins@kakwa.fr"'
              sh 'git config --global user.name "jenkins@kakwa.fr"'
              git 'https://github.com/kakwa/uts-server'
              sh 'git clean -fdx'
              sh 'cmake . -DBUNDLE_CIVETWEB=ON'
              sh 'make -j4'
            }
            
            
          }
        )
      }
    }
  }
  environment {
    GIT_SSL_NO_VERIFY = 'true'
    CIVETWEB_GITURL = 'https://gogs.kakwa.fr/kakwa/civetweb/'
  }
}