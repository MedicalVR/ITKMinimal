/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkRealToHalfHermitianForwardFFTImageFilter_hxx
#define itkRealToHalfHermitianForwardFFTImageFilter_hxx


namespace itk
{
template <typename TInputImage, typename TOutputImage>
RealToHalfHermitianForwardFFTImageFilter<TInputImage, TOutputImage>::RealToHalfHermitianForwardFFTImageFilter()
{
  this->SetActualXDimensionIsOdd(false);
}

template <typename TInputImage, typename TOutputImage>
void
RealToHalfHermitianForwardFFTImageFilter<TInputImage, TOutputImage>::GenerateOutputInformation()
{
  // Get pointers to the input and output.
  const typename InputImageType::ConstPointer inputPtr = this->GetInput();
  const typename OutputImageType::Pointer     outputPtr = this->GetOutput();

  if (!inputPtr || !outputPtr)
  {
    return;
  }

  // This is all based on the same function in itk::ShrinkImageFilter
  // ShrinkImageFilter also modifies the image spacing, but spacing
  // has no meaning in the result of an FFT.
  const InputSizeType  inputSize = inputPtr->GetLargestPossibleRegion().GetSize();
  const InputIndexType inputStartIndex = inputPtr->GetLargestPossibleRegion().GetIndex();

  OutputSizeType  outputSize;
  OutputIndexType outputStartIndex;

  // In 4.3.4 of the FFTW documentation, they indicate the size of
  // of a real-to-complex FFT is N * N ... + (N /2+1)
  //                              1   2        d
  // complex numbers.
  // static_cast probably not necessary but want to make sure integer
  // division is used.
  outputSize[0] = static_cast<unsigned int>(inputSize[0]) / 2 + 1;
  outputStartIndex[0] = inputStartIndex[0];

  for (unsigned int i = 1; i < OutputImageType::ImageDimension; ++i)
  {
    outputSize[i] = inputSize[i];
    outputStartIndex[i] = inputStartIndex[i];
  }

  const typename OutputImageType::RegionType outputLargestPossibleRegion(outputStartIndex, outputSize);

  outputPtr->SetLargestPossibleRegion(outputLargestPossibleRegion);
  this->SetActualXDimensionIsOdd(inputSize[0] % 2 != 0);
}

template <typename TInputImage, typename TOutputImage>
void
RealToHalfHermitianForwardFFTImageFilter<TInputImage, TOutputImage>::GenerateInputRequestedRegion()
{
  // Call the superclass implementation of this method.
  Superclass::GenerateInputRequestedRegion();

  // Get pointer to the input.
  const typename InputImageType::Pointer input = const_cast<InputImageType *>(this->GetInput());

  if (!input)
  {
    return;
  }

  input->SetRequestedRegionToLargestPossibleRegion();
}

template <typename TInputImage, typename TOutputImage>
void
RealToHalfHermitianForwardFFTImageFilter<TInputImage, TOutputImage>::EnlargeOutputRequestedRegion(DataObject * output)
{
  Superclass::EnlargeOutputRequestedRegion(output);
  output->SetRequestedRegionToLargestPossibleRegion();
}

template <typename TInputImage, typename TOutputImage>
SizeValueType
RealToHalfHermitianForwardFFTImageFilter<TInputImage, TOutputImage>::GetSizeGreatestPrimeFactor() const
{
  return 2;
}

} // namespace itk
#endif
