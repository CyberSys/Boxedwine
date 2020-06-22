#include "boxedwine.h"
#include "knativeaudio.h"
#include <SDL.h>
#include "../../source/kernel/devs/oss.h"

// Perhaps in the future, this class and devdsp.cpp will go away and instead I will replace the oss interface Wine uses in wineoss.drv with a custom one, like what I did with winex11.drv
#define DSP_BUFFER_SIZE (1024*256)

static bool sdlAudioOpen;
static U8 sdlSilence;

void closeSdlAudio() {
	if (sdlAudioOpen) {
		sdlAudioOpen = false;
		SDL_PauseAudio(1);
		SDL_CloseAudio();
	}
}

class KNativeAudioSdl : public KNativeAudio, public std::enable_shared_from_this<KNativeAudioSdl> {
public:
	KNativeAudioSdl() : bufferCond("KNativeAudioSdl::bufferCond") {
		memset(&this->want, 0, sizeof(this->want));
		memset(&this->got, 0, sizeof(this->got));
		this->cvtBufLen = 0;
		this->cvtBuf = NULL;
		this->cvtBufPos = 0;
		this->want.format = AUDIO_U8;
		this->want.channels = 1;
		this->want.freq = 11025;
		this->want.samples = 5512;
		this->got.channels = 1;
		this->got.freq = 11025;
		this->got.samples = 5512;
		this->sameFormat = false;
		this->open = false;
		this->closeWhenDone = false;
	}

	virtual ~KNativeAudioSdl() {
		if (this->cvtBuf) {
			SDL_free(this->cvtBuf);
		}
	}

	virtual void openAudio(U32 format, U32 freq, U32 channels);
	virtual bool isOpen() { return this->open; }
	virtual void closeAudio();
	virtual void writeAudio(U8* data, U32 len);
	virtual U32 getFragmentSize() {return this->got.samples;}
	virtual U32 getBufferSize() {return (U32)this->audioBuffer.size();}
	virtual U32 getBufferCapacity() { return DSP_BUFFER_SIZE;}

	void onClose();
	void closeAudioFromAudioThread();

	U32 bytesPerSampleWant() {
		if (this->want.format == AUDIO_S16LSB || this->want.format == AUDIO_S16MSB || this->want.format == AUDIO_U16LSB || this->want.format == AUDIO_U16MSB)
			return 2;
		else if (this->want.format == AUDIO_F32LSB)
			return 4;
		else
			return 1;
	}

	U32 getSdlFormat(U32 format) {
		switch (format) {
		case AFMT_MU_LAW:
		case AFMT_A_LAW:
		case AFMT_IMA_ADPCM:
		case AFMT_U8:
			return AUDIO_U8;		
		case AFMT_S16_LE:
			return AUDIO_S16LSB;
		case AFMT_S16_BE:
			return AUDIO_S16MSB;
		case AFMT_S8:
			return AUDIO_S8;
		case AFMT_U16_LE:
			return AUDIO_U16LSB;
		case AFMT_U16_BE:
			return AUDIO_U16MSB;
		case AFMT_MPEG:
			return AUDIO_U8;
		default:
			kpanic("KNativeAudioSdl Unknow audio format %d", format);
			return 0;
		}
	}
	SDL_AudioSpec want;
	SDL_AudioSpec got;
	SDL_AudioCVT cvt;
	int cvtBufLen;
	int cvtBufPos;
	unsigned char* cvtBuf;
	bool sameFormat;
	U32 dspFragSize;
	bool open;
	std::deque<U8> audioBuffer;
	BOXEDWINE_CONDITION bufferCond;
	bool closeWhenDone;
};

// not really a voice, currently they are not mixed
std::list<std::shared_ptr<KNativeAudioSdl>> voices;

void audioCallback(void* userdata, U8* stream, S32 len) {	
	if (!voices.size()) {
		memset(stream, sdlSilence, len);
		return;
	}
	std::shared_ptr<KNativeAudioSdl> data = voices.front();
	if (data->closeWhenDone && data->audioBuffer.size()==0 && (data->cvtBufPos == 0 || data->cvtBufPos >= data->cvt.len_cvt)) {
		data->closeAudioFromAudioThread();
		memset(stream, sdlSilence, len);
		return;
	}

	S32 available = (S32)data->audioBuffer.size();

	if (!data->sameFormat) {
		if (data->cvtBufPos < data->cvt.len_cvt) {
			S32 todo = data->cvt.len_cvt - data->cvtBufPos;
			if (todo > len)
				todo = len;
			memcpy(stream, data->cvt.buf + data->cvtBufPos, todo);
			data->cvtBufPos += todo;
			stream += todo;
			len -= todo;
		}
		if (len) {
			data->cvt.len = available;
			if (data->cvtBufLen && data->cvtBufLen < data->cvt.len * data->cvt.len_mult) {
				data->cvtBufLen = 0;
				SDL_free(data->cvtBuf);
				data->cvtBuf = NULL;
			}
			if (!data->cvtBufLen) {
				data->cvtBufLen = data->cvt.len * data->cvt.len_mult;
				data->cvtBuf = (Uint8*)SDL_malloc(data->cvt.len * data->cvt.len_mult);
			}
			data->cvt.buf = data->cvtBuf;

			std::copy(data->audioBuffer.begin(), data->audioBuffer.begin() + available, data->cvt.buf);
			data->audioBuffer.erase(data->audioBuffer.begin(), data->audioBuffer.begin() + available);

			SDL_ConvertAudio(&data->cvt);
			S32 todo = data->cvt.len_cvt;
			if (todo > len)
				todo = len;
			memcpy(stream, data->cvt.buf, todo);
			stream += todo;
			len -= todo;
			data->cvtBufPos = todo;
		}
	} else {
		if (available > len)
			available = len;
		if (available) {
			std::copy(data->audioBuffer.begin(), data->audioBuffer.begin() + available, stream);
			data->audioBuffer.erase(data->audioBuffer.begin(), data->audioBuffer.begin() + available);
			len -= available;
			stream += available;
		}
	}
	if (len) {
		memset(stream, data->got.silence, len);
	}

	BOXEDWINE_CONDITION_LOCK(data->bufferCond);
	BOXEDWINE_CONDITION_SIGNAL_ALL(data->bufferCond);
	BOXEDWINE_CONDITION_UNLOCK(data->bufferCond);
}

void KNativeAudioSdl::openAudio(U32 format, U32 freq, U32 channels) {
	//want.samples = 4096;    
	this->want.callback = audioCallback;
	this->want.format = getSdlFormat(format);
	this->want.freq = freq;
	this->want.channels = channels;

	if (!KSystem::soundEnabled) {
		this->sameFormat = true;
	} else {
		// If the previous audio is still playing, it will get cut off.  If I find a game that needs this, then perhaps I should think of a mixer.
		closeSdlAudio();
		if (SDL_OpenAudio(&this->want, &this->got) < 0) {
			printf("Failed to open audio: %s\n", SDL_GetError());
		}
		sdlSilence = this->got.silence;
		sdlAudioOpen = true;
		if (this->want.freq != this->got.freq || this->want.channels != this->got.channels || this->want.format != this->got.format) {
			this->sameFormat = false;
			SDL_BuildAudioCVT(&this->cvt, this->want.format, this->want.channels, this->want.freq, this->got.format, this->got.channels, this->got.freq);
		} else {
			this->sameFormat = true;
		}
	}
	this->open = true;
	voices.push_back(shared_from_this());
	if (KSystem::soundEnabled) {
		SDL_PauseAudio(0);
	}
	if (this->got.size) {
		this->dspFragSize = this->got.size;
	}
	printf("openAudio: freq=%d(got %d) format=%x(got %x) channels=%d(got %d)\n", this->want.freq, this->got.freq, this->want.format, this->got.format, this->want.channels, this->got.channels);
}

void KNativeAudioSdl::closeAudioFromAudioThread() {
	if (this->open) {
		this->onClose();
	}
}

void KNativeAudioSdl::closeAudio() {
	if (this->open) {
		bool needClose = true;
		if (KSystem::soundEnabled) {
			SDL_LockAudio();
		}
		if (audioBuffer.size() || (this->cvtBufPos != 0 && this->cvtBufPos < this->cvt.len_cvt)) {
			closeWhenDone = true;
			needClose = false;
		}
		if (KSystem::soundEnabled) {
			SDL_UnlockAudio();
		}
		if (needClose) {
			if (KSystem::soundEnabled) {
				closeSdlAudio();
			}
			this->onClose();
		}				
	}
}

void KNativeAudioSdl::onClose() {
	auto it = voices.begin();
	while (it != voices.end()) {
		std::shared_ptr<KNativeAudioSdl> p = *it;
		if (p == shared_from_this()) {
			it = voices.erase(it);
			break;
		} else {
			it++;
		}
	}
	this->open = false;
}

void KNativeAudioSdl::writeAudio(U8* data, U32 len) {
	SDL_LockAudio();
	BOXEDWINE_CONDITION_LOCK(this->bufferCond);
	audioBuffer.insert(this->audioBuffer.end(), data, data + len);
	BOXEDWINE_CONDITION_UNLOCK(this->bufferCond);
	SDL_UnlockAudio();
}

std::shared_ptr<KNativeAudio> KNativeAudio::createNativeAudio() {
	return std::make_shared<KNativeAudioSdl>();
}

void KNativeAudio::shutdown() {
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}
