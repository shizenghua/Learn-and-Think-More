**关于Fart的脱壳点：**

一个是Execute函数，另一个就是送到主动调用链的时候

- <clinit> - Execute => dumpDexFileByExecute
- 其他正常函数 - DexFile_dumpMethodCode => myfartInvoke => Invoke => dumpArtMethod







```
extern "C" void dumpDexFileByExecute(ArtMethod * artmethod)
	 SHARED_LOCKS_REQUIRED(Locks::mutator_lock_) {
		char *dexfilepath = (char *) malloc(sizeof(char) * 2000);
		if (dexfilepath == nullptr) {
			LOG(INFO) <<
			    "ArtMethod::dumpDexFileByExecute,methodname:"
			    << PrettyMethod(artmethod).
			    c_str() << "malloc 2000 byte failed";
			return;
		}
		int fcmdline = -1;
		char szCmdline[64] = { 0 };
		char szProcName[256] = { 0 };
		int procid = getpid();
		sprintf(szCmdline, "/proc/%d/cmdline", procid);
		fcmdline = open(szCmdline, O_RDONLY, 0644);
		if (fcmdline > 0) {
			read(fcmdline, szProcName, 256);
			close(fcmdline);
		}

		if (szProcName[0]) {

			const DexFile *dex_file = artmethod->GetDexFile();
			const uint8_t *begin_ = dex_file->Begin();	// Start of data.
			size_t size_ = dex_file->Size();	// Length of data.

			memset(dexfilepath, 0, 2000);
			int size_int_ = (int) size_;

			memset(dexfilepath, 0, 2000);
			sprintf(dexfilepath, "%s", "/sdcard/fart");
			mkdir(dexfilepath, 0777);

			memset(dexfilepath, 0, 2000);
			sprintf(dexfilepath, "/sdcard/fart/%s",
				szProcName);
			mkdir(dexfilepath, 0777);

			memset(dexfilepath, 0, 2000);
			sprintf(dexfilepath,
				"/sdcard/fart/%s/%d_dexfile_execute.dex",
				szProcName, size_int_);
			int dexfilefp = open(dexfilepath, O_RDONLY, 0666);
			if (dexfilefp > 0) {
				close(dexfilefp);
				dexfilefp = 0;

			} else {
				dexfilefp =
				    open(dexfilepath, O_CREAT | O_RDWR,
					 0666);
				if (dexfilefp > 0) {
					write(dexfilefp, (void *) begin_,
					      size_);
					fsync(dexfilefp);
					close(dexfilefp);
				}


			}


		}

		if (dexfilepath != nullptr) {
			free(dexfilepath);
			dexfilepath = nullptr;
		}

	}
	extern "C" void dumpArtMethod(ArtMethod * artmethod)
	 SHARED_LOCKS_REQUIRED(Locks::mutator_lock_) {
		char *dexfilepath = (char *) malloc(sizeof(char) * 2000);
		if (dexfilepath == nullptr) {
			LOG(INFO) <<
			    "ArtMethod::dumpArtMethodinvoked,methodname:"
			    << PrettyMethod(artmethod).
			    c_str() << "malloc 2000 byte failed";
			return;
		}
		int fcmdline = -1;
		char szCmdline[64] = { 0 };
		char szProcName[256] = { 0 };
		int procid = getpid();
		sprintf(szCmdline, "/proc/%d/cmdline", procid);
		fcmdline = open(szCmdline, O_RDONLY, 0644);
		if (fcmdline > 0) {
			read(fcmdline, szProcName, 256);
			close(fcmdline);
		}

		if (szProcName[0]) {

			const DexFile *dex_file = artmethod->GetDexFile();
			const char *methodname =
			    PrettyMethod(artmethod).c_str();
			const uint8_t *begin_ = dex_file->Begin();
			size_t size_ = dex_file->Size();

			memset(dexfilepath, 0, 2000);
			int size_int_ = (int) size_;

			memset(dexfilepath, 0, 2000);
			sprintf(dexfilepath, "%s", "/sdcard/fart");
			mkdir(dexfilepath, 0777);

			memset(dexfilepath, 0, 2000);
			sprintf(dexfilepath, "/sdcard/fart/%s",
				szProcName);
			mkdir(dexfilepath, 0777);

			memset(dexfilepath, 0, 2000);
			sprintf(dexfilepath,
				"/sdcard/fart/%s/%d_dexfile.dex",
				szProcName, size_int_);
			int dexfilefp = open(dexfilepath, O_RDONLY, 0666);
			if (dexfilefp > 0) {
				close(dexfilefp);
				dexfilefp = 0;

			} else {
				dexfilefp =
				    open(dexfilepath, O_CREAT | O_RDWR,
					 0666);
				if (dexfilefp > 0) {
					write(dexfilefp, (void *) begin_,
					      size_);
					fsync(dexfilefp);
					close(dexfilefp);
				}


			}
			const DexFile::CodeItem * code_item =
			    artmethod->GetCodeItem();
			if (LIKELY(code_item != nullptr)) {
				int code_item_len = 0;
				uint8_t *item = (uint8_t *) code_item;
				if (code_item->tries_size_ > 0) {
					const uint8_t *handler_data =
					    (const uint8_t *) (DexFile::
							       GetTryItems
							       (*code_item,
								code_item->
								tries_size_));
					uint8_t *tail =
					    codeitem_end(&handler_data);
					code_item_len =
					    (int) (tail - item);
				} else {
					code_item_len =
					    16 +
					    code_item->
					    insns_size_in_code_units_ * 2;
				}
				memset(dexfilepath, 0, 2000);
				int size_int = (int) dex_file->Size();	// Length of data
				uint32_t method_idx =
				    artmethod->get_method_idx();
				sprintf(dexfilepath,
					"/sdcard/fart/%s/%d_%ld.bin",
					szProcName, size_int, gettidv1());
				int fp2 =
				    open(dexfilepath,
					 O_CREAT | O_APPEND | O_RDWR,
					 0666);
				if (fp2 > 0) {
					lseek(fp2, 0, SEEK_END);
					memset(dexfilepath, 0, 2000);
					int offset = (int) (item - begin_);
					sprintf(dexfilepath,
						"{name:%s,method_idx:%d,offset:%d,code_item_len:%d,ins:",
						methodname, method_idx,
						offset, code_item_len);
					int contentlength = 0;
					while (dexfilepath[contentlength]
					       != 0)
						contentlength++;
					write(fp2, (void *) dexfilepath,
					      contentlength);
					long outlen = 0;
					char *base64result =
					    base64_encode((char *) item,
							  (long)
							  code_item_len,
							  &outlen);
					write(fp2, base64result, outlen);
					write(fp2, "};", 2);
					fsync(fp2);
					close(fp2);
					if (base64result != nullptr) {
						free(base64result);
						base64result = nullptr;
					}
				}

			}


		}

		if (dexfilepath != nullptr) {
			free(dexfilepath);
			dexfilepath = nullptr;
		}

	}
	extern "C" void myfartInvoke(ArtMethod * artmethod)
	 SHARED_LOCKS_REQUIRED(Locks::mutator_lock_) {
		JValue *result = nullptr;
		Thread *self = nullptr;
		uint32_t temp = 6;
		uint32_t *args = &temp;
		uint32_t args_size = 6;
		artmethod->Invoke(self, args, args_size, result, "fart");
	}

```





```
	void ArtMethod::Invoke(Thread * self, uint32_t * args,
			       uint32_t args_size, JValue * result,
			       const char *shorty) {


		if (self == nullptr) {
			dumpArtMethod(this);
			return;
		}
		if (UNLIKELY
		    (__builtin_frame_address(0) < self->GetStackEnd())) {
			ThrowStackOverflowError(self);
			return;
		}

		if (kIsDebugBuild) {
			self->AssertThreadSuspensionIsAllowable();
			CHECK_EQ(kRunnable, self->GetState());
			CHECK_STREQ(GetInterfaceMethodIfProxy
				    (sizeof(void *))->GetShorty(), shorty);
		}
		// Push a transition back into managed code onto the linked list in thread.
		ManagedStack fragment;
		self->PushManagedStackFragment(&fragment);

		Runtime *runtime = Runtime::Current();
		// Call the invoke stub, passing everything as arguments.
		// If the runtime is not yet started or it is required by the debugger, then perform the
		// Invocation by the interpreter.
		if (UNLIKELY
		    (!runtime->IsStarted()
		     || Dbg::IsForcedInterpreterNeededForCalling(self,
								 this))) {
			if (IsStatic()) {
				art::interpreter::
				    EnterInterpreterFromInvoke(self, this,
							       nullptr,
							       args,
							       result);
			} else {
				mirror::Object * receiver =
				    reinterpret_cast < StackReference <
				    mirror::Object >
				    *>(&args[0])->AsMirrorPtr();
				art::interpreter::
				    EnterInterpreterFromInvoke(self, this,
							       receiver,
							       args + 1,
							       result);
			}
		} else {
			DCHECK_EQ(runtime->GetClassLinker()->
				  GetImagePointerSize(), sizeof(void *));

			constexpr bool kLogInvocationStartAndReturn =
			    false;
			bool have_quick_code =
			    GetEntryPointFromQuickCompiledCode() !=
			    nullptr;
			if (LIKELY(have_quick_code)) {
				if (kLogInvocationStartAndReturn) {
					LOG(INFO) <<
					    StringPrintf
					    ("Invoking '%s' quick code=%p static=%d",
					     PrettyMethod(this).c_str(),
					     GetEntryPointFromQuickCompiledCode
					     (),
					     static_cast <
					     int >(IsStatic()? 1 : 0));
				}
				// Ensure that we won't be accidentally calling quick compiled code when -Xint.
				if (kIsDebugBuild
				    && runtime->GetInstrumentation()->
				    IsForcedInterpretOnly()) {
					DCHECK(!runtime->UseJit());
					CHECK(IsEntrypointInterpreter())
					    <<
					    "Don't call compiled code when -Xint "
					    << PrettyMethod(this);
				}
#if defined(__LP64__) || defined(__arm__) || defined(__i386__)
				if (!IsStatic()) {
					(*art_quick_invoke_stub) (this,
								  args,
								  args_size,
								  self,
								  result,
								  shorty);
				} else {
					(*art_quick_invoke_static_stub)
					    (this, args, args_size, self,
					     result, shorty);
				}
#else
				(*art_quick_invoke_stub) (this, args,
							  args_size, self,
							  result, shorty);
#endif
				if (UNLIKELY
				    (self->GetException() ==
				     Thread::
				     GetDeoptimizationException())) {
					// Unusual case where we were running generated code and an
					// exception was thrown to force the activations to be removed from the
					// stack. Continue execution in the interpreter.
					self->ClearException();
					ShadowFrame *shadow_frame =
					    self->
					    PopStackedShadowFrame
					    (StackedShadowFrameType::
					     kDeoptimizationShadowFrame);
					result->SetJ(self->
						     PopDeoptimizationReturnValue
						     ().GetJ());
					self->SetTopOfStack(nullptr);
					self->
					    SetTopOfShadowStack
					    (shadow_frame);
					interpreter::
					    EnterInterpreterFromDeoptimize
					    (self, shadow_frame, result);
				}
				if (kLogInvocationStartAndReturn) {
					LOG(INFO) <<
					    StringPrintf
					    ("Returned '%s' quick code=%p",
					     PrettyMethod(this).c_str(),
					     GetEntryPointFromQuickCompiledCode
					     ());
				}
			} else {
				LOG(INFO) << "Not invoking '" <<
				    PrettyMethod(this) << "' code=null";
				if (result != nullptr) {
					result->SetJ(0);
				}
			}
		}

		// Pop transition.
		self->PopManagedStackFragment(fragment);
	}

```

