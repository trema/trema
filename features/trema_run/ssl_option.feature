Feature: SSL option
  Before run this script, setup a SSL enabled openflow switch by using
  certificates under the spec/fixtures/certs directory.
  ex)
    This is an open-vswitch example.
    $ sudo ovs-vsctl set-ssl certs/sc-privkey.pem certs/sc-cert.pem certs/cacert.pem
    $ sudo ovs-vsctl set bridge br0 other_config:datapath-id=0000000000000001
    $ sudo ovs-vsctl set-controller br0 ssl:hostname:6653

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "hello.rb" with:
      """ruby
      class Hello < Trema::Controller
        def switch_ready(dpid)
          logger.info format('Hello %s!', dpid.to_hex)
        end
      end
      """
    And a file named "ctl-cert.pem" with:
      """
      Certificate:
          Data:
              Version: 1 (0x0)
              Serial Number: 2 (0x2)
          Signature Algorithm: md5WithRSAEncryption
              Issuer: C=US, ST=CA, O=Open vSwitch, OU=controllerca, CN=OVS controllerca CA Certificate (2016  4\xE6\x9C\x88 10 11:22:15)
              Validity
                  Not Before: Apr 10 02:23:22 2016 GMT
                  Not After : Apr  8 02:23:22 2026 GMT
              Subject: C=US, ST=CA, O=Open vSwitch, OU=Open vSwitch certifier, CN=ctl id:dfccc23a-c7ba-4622-a279-98f3139f9171
              Subject Public Key Info:
                  Public Key Algorithm: rsaEncryption
                      Public-Key: (2048 bit)
                      Modulus:
                          00:d5:ee:ba:f1:04:77:63:75:00:e2:32:64:f1:be:
                          9b:2f:fc:86:a6:2b:d9:ab:a7:d6:a1:c1:ef:76:9f:
                          9b:c8:90:e7:8a:67:e9:08:dc:db:6a:af:69:20:9f:
                          de:ae:8c:dc:f1:fe:9a:54:8e:2e:9f:31:27:b8:7e:
                          ce:24:d0:9a:a9:4c:0f:8f:ac:86:e0:62:53:b9:fd:
                          ca:42:06:f1:51:0e:41:5f:48:99:3c:5d:29:cc:16:
                          6e:81:4a:9f:b7:8b:f2:2c:82:e3:85:d7:43:b1:26:
                          1a:ba:21:fd:f1:0e:21:e2:a4:58:12:af:ff:d1:27:
                          6b:06:42:dc:a6:25:a1:c0:b4:ac:90:18:91:e1:13:
                          80:79:78:3e:c2:f2:3c:7f:49:5b:61:a9:3b:0f:49:
                          3d:d0:79:d1:1a:e1:a9:78:64:70:39:58:7f:d4:09:
                          f7:6e:47:67:4a:9f:ba:a7:97:93:82:92:39:51:32:
                          3f:20:cd:30:9d:ea:5b:75:e3:93:94:1a:e2:34:b8:
                          da:38:b5:ae:94:49:99:fd:3d:60:11:0b:ae:ff:4e:
                          66:93:cd:fd:f7:25:f9:2d:b2:82:3b:4a:b1:1b:b5:
                          ee:8d:40:c2:d9:a1:b0:5b:ed:43:c2:15:29:f6:93:
                          4c:a3:08:be:35:13:fc:31:a1:14:20:f9:81:6e:28:
                          b2:99
                      Exponent: 65537 (0x10001)
          Signature Algorithm: md5WithRSAEncryption
               26:ca:35:85:83:e7:b8:2a:f3:24:11:07:ff:9f:e7:9f:ed:a0:
               0e:ee:ba:b0:ed:c2:a7:18:5c:97:c4:30:5d:88:d6:ad:63:27:
               68:74:49:2e:8b:c4:a9:d2:28:35:b9:6e:e1:f0:56:79:ac:48:
               52:08:52:14:33:7b:bd:75:12:15:0f:97:cb:af:92:64:b5:4d:
               a8:d6:92:21:a5:54:57:e5:da:dd:2b:48:88:63:ce:2d:64:e9:
               32:a8:da:f7:f2:66:32:04:6d:54:10:06:a3:0b:c7:b8:ee:6d:
               b3:82:42:c8:56:df:4f:fd:cf:45:a5:e8:f0:2e:24:3f:9b:e9:
               b8:e5:6d:01:87:e8:44:da:5e:8e:1e:10:6a:8f:c8:7f:7e:a1:
               0e:9e:c5:e0:bd:d2:d6:89:8f:1d:ad:fb:3e:6b:86:92:bb:3d:
               95:13:37:74:5d:08:b3:44:50:01:ab:ee:46:12:7b:05:90:d9:
               be:bc:50:bc:5a:35:08:6d:cd:47:7a:71:35:fe:1a:4a:66:35:
               eb:8e:54:cf:45:5a:7b:c9:c8:cb:97:4b:10:66:e2:ad:ef:19:
               2b:ba:b0:c3:ad:16:7d:e0:7e:52:db:c0:90:2c:34:08:e8:9c:
               38:62:0d:a1:52:6e:f4:6e:b6:c5:4d:c4:9a:8f:95:99:f3:b7:
               d5:c1:41:ed
      -----BEGIN CERTIFICATE-----
      MIIDiTCCAnECAQIwDQYJKoZIhvcNAQEEBQAwgYsxCzAJBgNVBAYTAlVTMQswCQYD
      VQQIEwJDQTEVMBMGA1UEChMMT3BlbiB2U3dpdGNoMRUwEwYDVQQLEwxjb250cm9s
      bGVyY2ExQTA/BgNVBAMUOE9WUyBjb250cm9sbGVyY2EgQ0EgQ2VydGlmaWNhdGUg
      KDIwMTYgIDTmnIggMTAgMTE6MjI6MTUpMB4XDTE2MDQxMDAyMjMyMloXDTI2MDQw
      ODAyMjMyMlowgYgxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTEVMBMGA1UEChMM
      T3BlbiB2U3dpdGNoMR8wHQYDVQQLExZPcGVuIHZTd2l0Y2ggY2VydGlmaWVyMTQw
      MgYDVQQDEytjdGwgaWQ6ZGZjY2MyM2EtYzdiYS00NjIyLWEyNzktOThmMzEzOWY5
      MTcxMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1e668QR3Y3UA4jJk
      8b6bL/yGpivZq6fWocHvdp+byJDnimfpCNzbaq9pIJ/erozc8f6aVI4unzEnuH7O
      JNCaqUwPj6yG4GJTuf3KQgbxUQ5BX0iZPF0pzBZugUqft4vyLILjhddDsSYauiH9
      8Q4h4qRYEq//0SdrBkLcpiWhwLSskBiR4ROAeXg+wvI8f0lbYak7D0k90HnRGuGp
      eGRwOVh/1An3bkdnSp+6p5eTgpI5UTI/IM0wnepbdeOTlBriNLjaOLWulEmZ/T1g
      EQuu/05mk8399yX5LbKCO0qxG7XujUDC2aGwW+1DwhUp9pNMowi+NRP8MaEUIPmB
      biiymQIDAQABMA0GCSqGSIb3DQEBBAUAA4IBAQAmyjWFg+e4KvMkEQf/n+ef7aAO
      7rqw7cKnGFyXxDBdiNatYydodEkui8Sp0ig1uW7h8FZ5rEhSCFIUM3u9dRIVD5fL
      r5JktU2o1pIhpVRX5drdK0iIY84tZOkyqNr38mYyBG1UEAajC8e47m2zgkLIVt9P
      /c9FpejwLiQ/m+m45W0Bh+hE2l6OHhBqj8h/fqEOnsXgvdLWiY8drfs+a4aSuz2V
      Ezd0XQizRFABq+5GEnsFkNm+vFC8WjUIbc1HenE1/hpKZjXrjlTPRVp7ycjLl0sQ
      ZuKt7xkrurDDrRZ94H5S28CQLDQI6Jw4Yg2hUm70brbFTcSaj5WZ87fVwUHt
      -----END CERTIFICATE-----
      """
    And a file named "ctl-privkey.pem" with:
      """
      -----BEGIN RSA PRIVATE KEY-----
      MIIEowIBAAKCAQEA1e668QR3Y3UA4jJk8b6bL/yGpivZq6fWocHvdp+byJDnimfp
      CNzbaq9pIJ/erozc8f6aVI4unzEnuH7OJNCaqUwPj6yG4GJTuf3KQgbxUQ5BX0iZ
      PF0pzBZugUqft4vyLILjhddDsSYauiH98Q4h4qRYEq//0SdrBkLcpiWhwLSskBiR
      4ROAeXg+wvI8f0lbYak7D0k90HnRGuGpeGRwOVh/1An3bkdnSp+6p5eTgpI5UTI/
      IM0wnepbdeOTlBriNLjaOLWulEmZ/T1gEQuu/05mk8399yX5LbKCO0qxG7XujUDC
      2aGwW+1DwhUp9pNMowi+NRP8MaEUIPmBbiiymQIDAQABAoIBAE47dQV6WqZ2PRJ6
      10tIJrwPnrXZx0nsoKKapxU8HN3lj6afhSqGiX6kEs+pZudx/8JHFuzg6c+xTBM9
      2+i+mDBc7jveHZykmHWlh3dJzqmTivhrNg5LC2PkuBhzz6BxfugkHUvugoSfqJp0
      n8atIlsdOk/rKKO1xH+Pp9ziWhp7zHhkbtt5Gg/oKcMMt3NoynGN8GQyKkbWSz8S
      NKhXTzC+n9jAgq503cVi+XLo49gcUTl6cGumwFh2LBw17jBKqJRnAslGw5z6HuVF
      gtW7vg59H1h6nCvn5w69raEwSYgKMGOhEgBlpXr/VhGMjxLfYQ8ZnsN8nFxumP3k
      onm5CGECgYEA8XlTFuSb1u4iTsB/MOAPtjjAf33JC3DF3jBIxnfZfkS4gdOPe4Vs
      t9kCTDlhzXSuGdqvuGxrc1RXTnXrQByXgxOm1RbWas84V8ItjGR+vQc7izg07ZUz
      EW2hWi/rML1FttX0fnUncffj78JK2OueLpkKzMQ37Y2uU3TNT2z3d70CgYEA4s1F
      XvVKKawgXHWofeT/AO8jSnfQ1tzjNEu41AU4w65ykR5G4CibqQgy8j7CBIK/jjbJ
      LTufSbf/UL1hFr/G9Otd3pbdLtYTYASffvQ9mmA1Z1Cji+w8ELgBTPkLQdATBmkn
      2J5rWsavT9heJWG10VauFmNX1um7noy1Qp8o9g0CgYAsnMmpFRtlniFgg1f8Y6kD
      dYC7DTYzkuY7opgPct0P4wpjkf6UR5ZKcSFni8Jx/vibdje4zu8bI53ttQN9mMq1
      QNA11j5PBXHXZNydb4Oq0MdDdWLx8fq5YZPJ4ciF/LIAkY7WPjbHa5EFHtRNN1rG
      KShBpI7gqKhoas/zuKIP0QKBgQDMDgugB+kV3TuwdFuICYQ0/vMtiOdoqYvx/T7p
      41jTFh0V27vA6khCqJHNyhEdpdVayofuHnqOBPG9XuX/ZcRHj4wBPJL8FLeR6Bbo
      Ti8uuejSb4b12TGhiSwCaP9r0x2K4wlqp/3GyoPovq7VyzeehPJUSkU5o0meuNJM
      go6D8QKBgCEHx2EYqmagf5YnykiQ3eN0A7q+uhkunRtIPGk7im4a34aFTR8ND5M/
      +dDgD2L5HMexwRkni8Z7VI1zYAe7f+nzzNA+J3z5ln1TgUqu+GPLt7VmSCb0aHkH
      uSUzoQ7RF7QSog9OMd80PzYymLvJwezZBSlwdxt9j2ev8klm4+AH
      -----END RSA PRIVATE KEY-----
      """
    And a file named "cacert.pem" with:
      """
      Certificate:
          Data:
              Version: 1 (0x0)
              Serial Number: 1 (0x1)
          Signature Algorithm: md5WithRSAEncryption
              Issuer: C=US, ST=CA, O=Open vSwitch, OU=controllerca, CN=OVS controllerca CA Certificate (2016  4\xE6\x9C\x88 10 11:22:15)
              Validity
                  Not Before: Apr 10 02:22:15 2016 GMT
                  Not After : Apr  8 02:22:15 2026 GMT
              Subject: C=US, ST=CA, O=Open vSwitch, OU=controllerca, CN=OVS controllerca CA Certificate (2016  4\xE6\x9C\x88 10 11:22:15)
              Subject Public Key Info:
                  Public Key Algorithm: rsaEncryption
                      Public-Key: (2048 bit)
                      Modulus:
                          00:af:bd:70:59:de:83:18:c2:27:9b:a6:4c:ae:72:
                          79:92:e4:a7:93:4a:fc:37:f2:71:3a:8c:a0:d5:c4:
                          33:c1:87:63:33:3d:91:6e:1e:f3:74:61:b5:a2:16:
                          fe:c6:68:1f:d6:1a:2e:ce:69:3f:9b:27:83:06:4d:
                          77:82:d1:e2:71:8f:18:07:be:07:76:15:14:a5:00:
                          0b:df:1f:50:92:aa:78:9e:4d:ab:c0:5c:6d:9f:e0:
                          12:f3:bb:b0:58:9b:13:c3:d9:43:ee:20:20:1f:73:
                          b9:77:f2:51:6c:d0:5a:8f:6a:a8:1c:cc:0f:2d:46:
                          38:e6:13:86:78:a4:0b:7e:9a:ec:4e:58:f1:71:ca:
                          e4:92:b3:14:3c:8d:b2:0c:23:b3:cd:0d:4b:21:eb:
                          16:e3:30:06:66:a3:42:1b:ef:75:f0:d8:20:72:c8:
                          e6:7c:02:e8:c9:d8:00:d8:5c:87:bc:4e:cb:e2:7e:
                          7e:ad:62:de:5d:ab:20:20:35:30:76:38:e4:ce:68:
                          ca:76:55:46:f5:9e:ee:e6:db:3a:57:c9:5f:63:37:
                          48:d9:8c:27:5e:61:fe:c8:29:7a:58:64:ec:5e:2a:
                          9d:d3:60:7b:e9:af:d0:83:73:b8:66:b9:0f:f4:b5:
                          6a:6f:70:7a:0d:c5:42:54:53:79:6e:cb:82:4c:5d:
                          d4:77
                      Exponent: 65537 (0x10001)
          Signature Algorithm: md5WithRSAEncryption
               4a:ea:ad:0c:a5:6f:86:da:b6:b7:b1:d5:ae:1e:97:07:d5:cf:
               60:1b:7c:21:51:5b:6e:a0:ed:02:0f:d3:2c:c3:9b:9a:45:24:
               1d:db:36:da:b7:bb:09:d9:ab:ea:0b:69:35:29:da:11:58:8c:
               0e:22:8d:cd:86:1e:9e:11:a6:90:e3:b5:08:04:a8:a8:96:66:
               9f:11:35:62:e5:93:00:e8:d8:65:28:ba:b8:d6:92:4d:70:ac:
               20:2b:ed:02:05:9a:f4:36:35:e8:1a:d2:96:7b:02:a4:8e:81:
               7d:e5:8b:0d:1f:3a:af:78:a9:c5:ec:66:cc:9a:89:8b:a9:c5:
               76:9a:76:bc:b2:75:a8:9a:e7:95:49:33:51:90:8f:49:4e:8f:
               84:6b:e4:25:6c:2f:71:34:62:4a:bf:1e:47:25:68:04:6c:be:
               8a:3e:f5:54:b2:ad:59:b6:12:28:b5:95:a5:21:1a:a0:a5:2d:
               56:3a:fa:cf:8d:d7:77:48:c6:d7:d5:ac:99:86:8f:e7:f1:cc:
               d1:79:f2:17:b6:f8:05:76:c8:ab:39:8d:c1:94:ac:49:20:c4:
               7f:5d:ec:12:c6:59:ed:d9:26:d8:ca:3a:7e:a2:2f:1a:a0:d0:
               69:26:fe:8c:29:88:5c:16:de:20:4e:19:d8:ae:9b:e3:3f:c7:
               19:c2:fa:5d
      -----BEGIN CERTIFICATE-----
      MIIDjDCCAnQCAQEwDQYJKoZIhvcNAQEEBQAwgYsxCzAJBgNVBAYTAlVTMQswCQYD
      VQQIEwJDQTEVMBMGA1UEChMMT3BlbiB2U3dpdGNoMRUwEwYDVQQLEwxjb250cm9s
      bGVyY2ExQTA/BgNVBAMUOE9WUyBjb250cm9sbGVyY2EgQ0EgQ2VydGlmaWNhdGUg
      KDIwMTYgIDTmnIggMTAgMTE6MjI6MTUpMB4XDTE2MDQxMDAyMjIxNVoXDTI2MDQw
      ODAyMjIxNVowgYsxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTEVMBMGA1UEChMM
      T3BlbiB2U3dpdGNoMRUwEwYDVQQLEwxjb250cm9sbGVyY2ExQTA/BgNVBAMUOE9W
      UyBjb250cm9sbGVyY2EgQ0EgQ2VydGlmaWNhdGUgKDIwMTYgIDTmnIggMTAgMTE6
      MjI6MTUpMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAr71wWd6DGMIn
      m6ZMrnJ5kuSnk0r8N/JxOoyg1cQzwYdjMz2Rbh7zdGG1ohb+xmgf1houzmk/myeD
      Bk13gtHicY8YB74HdhUUpQAL3x9Qkqp4nk2rwFxtn+AS87uwWJsTw9lD7iAgH3O5
      d/JRbNBaj2qoHMwPLUY45hOGeKQLfprsTljxccrkkrMUPI2yDCOzzQ1LIesW4zAG
      ZqNCG+918NggcsjmfALoydgA2FyHvE7L4n5+rWLeXasgIDUwdjjkzmjKdlVG9Z7u
      5ts6V8lfYzdI2YwnXmH+yCl6WGTsXiqd02B76a/Qg3O4ZrkP9LVqb3B6DcVCVFN5
      bsuCTF3UdwIDAQABMA0GCSqGSIb3DQEBBAUAA4IBAQBK6q0MpW+G2ra3sdWuHpcH
      1c9gG3whUVtuoO0CD9Msw5uaRSQd2zbat7sJ2avqC2k1KdoRWIwOIo3Nhh6eEaaQ
      47UIBKiolmafETVi5ZMA6NhlKLq41pJNcKwgK+0CBZr0NjXoGtKWewKkjoF95YsN
      HzqveKnF7GbMmomLqcV2mna8snWomueVSTNRkI9JTo+Ea+QlbC9xNGJKvx5HJWgE
      bL6KPvVUsq1ZthIotZWlIRqgpS1WOvrPjdd3SMbX1ayZho/n8czRefIXtvgFdsir
      OY3BlKxJIMR/XewSxlnt2SbYyjp+oi8aoNBpJv6MKYhcFt4gThnYrpvjP8cZwvpd
      -----END CERTIFICATE-----
      """

  @sudo
  Scenario: SSL connection
    When I run `trema run hello.rb --ctl_privkey=./ctl-privkey.pem --ctl_cert=./ctl-cert.pem --ca_certs=./cacert.pem -d`
    And sleep 10
    And the file "Hello.log" should contain "Hello 0x1!"
