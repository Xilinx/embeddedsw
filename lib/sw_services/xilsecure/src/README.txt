Copyright (C) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.


xilsecure library has 3 directories in xilsecure/src/
	1. server - It contains server interface APIs for AES, RSA, ECDSA and SHA crypto engines
	2. client - It contains client interface APIs for AES, RSA, ECDSA and SHA crypto engines
	3. common - It contains common APIs between client and server


  1. server has 4 directories
	a. core - Common core features which are applicable different platforms.
		|
		---aes directory contains AES core interface APIs and respective IPI handlers.
		|
		---crypto_kat directory contains KAT core APIs and respective IPI handlers.
		|
		---ecc_keypair directory contains ECC key pair generation APIs.
		|
		---ecdsa directory contains ECDSA core interface APIs and respective IPI handlers.
		|
		---generic directory contains xilsecure init and EXPORT complaince related APIs.
		|
		---hmac directory contains HMAC core APIs
		|
		---key_unwrap directory contains key_unwrap implementation APIs
		|
		---key_zeroize directory contains AES key zeroization APIs
		|
		---rsa directory contains RSA core interface APIs and respective IPI handlers.
		|  |
		|  --- rsa_qmode contains RSA QUIET MODE implementation APIs - Applicable for VersalNet and Versal_2Ve_2Vm
		|
		---sha directory contains SHA core interface APIs and respective IPI handlers.
		|  |
		|  ----- sha_pmx - Applicable for Versal and VersalNet
		|  |
		|  ----- sha_pmxc - Applicable for Versal_2Ve_2Vm
		|
		---softsha2-384 contains SOFT SHA2-384 APIs
		|
		---trng directory contains TRNG APIs
		|
		---util directory contains utility APIs for SSS.
	b. versal - Versal platform specific APIs
	c. versal_net - VersalNet platform specific APIs
	d. versal_2ve_2vm - Versal_2Ve_2Vm platform specific APIs
	e. zynqmp - ZynqMp platform specific APIs
	f. spartanup - SpartanUp platform specific APIs

  2. common has 4 directories
	a. core - Common core features which are applicable different platforms.
			Required features need to be included required platforms.
	b. versal - Versal platform specific APIs
	c. versal_net - VersalNet platform specific APIs
	d. versal_2ve_2vm - Versal_2Ve_2Vm platform specific APIs

  3. client has 2 directories
	a. core - Common core features which are applicable different platforms.
		|
		---aes directory contains AES client interface APIs.
		|
		---crypto_kat directory contains KAT client APIs.
		|
		---ecc_keypair directory contains ECC key pair generation client APIs.
		|
		---ecdsa directory contains ECDSA core interface APIs.
		|
		---key_zeroize directory contains AES key zeroization client APIs
		|
		---rsa directory contains RSA client interface APIs.
		|
		---sha directory contains SHA client interface APIs.
		|  |
		|  ----- sha_pmx - Applicable for Versal and VersalNet
		|  |
		|  ----- sha_pmxc - Applicable for Versal_2Ve_2Vm and SpartanUp
		|
		---trng directory contains TRNG client interface APIs.
		|
		---mailbox directory contains mailbox helper APIs to send/receive IPI requests.

	b. versal_net - VersalNet platform specific APIs

core features are applicable for different platforms as below
  server
   |
   --- Versal - aes, sha/sha_pmx, rsa(rsa_qmode is NA), ecdsa, crypto_kat, generic and util features are applicable.
   |
   --- VersalNet - aes, sha/sha_pmx, rsa, ecdsa, crypto_kat, generic, util, hmac, key_unwrap, ecc_keypai,
   |               key_zeroize, trng and softsha2-384 features are applicable
   |
   --- Versal_2Ve_2Vm - aes, sha/sha_pmxc, rsa, ecdsa, crypto_kat, generic, util, hmac, key_unwrap, ecc_keypai,
   |                   key_zeroize, trng and softsha2-384 features are applicable
   |
   --- SpartanUp - aes and sha/sha_pmxc

  client
   |
   --- Versal - aes, sha/sha_pmx, rsa, ecdsa, crypto_kat and mailbox features are applicable
   |
   --- VersalNet - aes, sha/sha_pmx, rsa, ecdsa, crypto_kat, ecc_keypair,
   |               key_zeroize and trng features are applicable
   |
   --- Versal_2Ve_2Vm - aes, sha/sha_pmxc, rsa, ecdsa, crypto_kat, ecc_keypair,
                      key_zeroize and trng features are applicable
