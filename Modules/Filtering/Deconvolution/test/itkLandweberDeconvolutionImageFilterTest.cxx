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

#include "itkFFTConvolutionImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkLandweberDeconvolutionImageFilter.h"
#include "itkDeconvolutionIterationCommand.h"
#include "itkSimpleFilterWatcher.h"
#include "itkMacro.h"
#include "itkMath.h"
#include "itkTestingMacros.h"

int
itkLandweberDeconvolutionImageFilterTest(int argc, char * argv[])
{
  if (argc < 5)
  {
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv)
              << " <input image> <kernel image> <output image> <iterations> [convolution image]" << std::endl;
    return EXIT_FAILURE;
  }

  using PixelType = float;
  constexpr unsigned int Dimension = 2;
  using ImageType = itk::Image<PixelType, Dimension>;
  using ReaderType = itk::ImageFileReader<ImageType>;
  using WriterType = itk::ImageFileWriter<ImageType>;

  auto inputReader = ReaderType::New();
  inputReader->SetFileName(argv[1]);
  inputReader->Update();

  auto kernelReader = ReaderType::New();
  kernelReader->SetFileName(argv[2]);
  kernelReader->Update();

  // Generate a convolution of the input image with the kernel image
  using ConvolutionFilterType = itk::FFTConvolutionImageFilter<ImageType>;
  auto convolutionFilter = ConvolutionFilterType::New();
  convolutionFilter->SetInput(inputReader->GetOutput());
  convolutionFilter->NormalizeOn();
  convolutionFilter->SetKernelImage(kernelReader->GetOutput());

  // Test the deconvolution algorithm
  using DeconvolutionFilterType = itk::LandweberDeconvolutionImageFilter<ImageType>;
  auto deconvolutionFilter = DeconvolutionFilterType::New();
  deconvolutionFilter->SetInput(convolutionFilter->GetOutput());
  deconvolutionFilter->SetKernelImage(kernelReader->GetOutput());
  deconvolutionFilter->NormalizeOn();
  deconvolutionFilter->SetAlpha(std::stod(argv[5]));
  if (itk::Math::NotExactlyEquals(deconvolutionFilter->GetAlpha(), std::stod(argv[5])))
  {
    std::cerr << "Set/GetAlpha() test failed." << std::endl;
    return EXIT_FAILURE;
  }

  auto iterations = static_cast<unsigned int>(std::stoi(argv[4]));
  deconvolutionFilter->SetNumberOfIterations(iterations);

  // Add an observer to report on filter iteration progress
  using IterationCommandType = itk::DeconvolutionIterationCommand<DeconvolutionFilterType>;
  auto observer = IterationCommandType::New();
  deconvolutionFilter->AddObserver(itk::IterationEvent(), observer);

  const itk::SimpleFilterWatcher watcher(deconvolutionFilter);

  // Write the deconvolution result
  try
  {
    auto writer = WriterType::New();
    writer->SetFileName(argv[3]);
    writer->SetInput(deconvolutionFilter->GetOutput());
    writer->Update();
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cerr << "Unexpected exception caught when writing deconvolution image: " << e << std::endl;
    return EXIT_FAILURE;
  }

  if (!observer->GetInvoked())
  {
    std::cerr << "Iteration command observer was never invoked, but should have been." << std::endl;
    return EXIT_FAILURE;
  }

  deconvolutionFilter->Print(std::cout);

  // Instantiate types with non-default template parameters
  using FloatImageType = itk::Image<float, Dimension>;
  using DoubleImageType = itk::Image<double, Dimension>;
  using IntImageType = itk::Image<int, Dimension>;

  using FilterType = itk::LandweberDeconvolutionImageFilter<FloatImageType, DoubleImageType, IntImageType, float>;
  auto filter = FilterType::New();
  filter->Print(std::cout);

  return EXIT_SUCCESS;
}
